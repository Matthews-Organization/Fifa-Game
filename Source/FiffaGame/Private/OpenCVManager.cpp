// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenCVManager.h"

// Sets default values
AOpenCVManager::AOpenCVManager()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOpenCVManager::BeginPlay()
{
    Super::BeginPlay();
    
    Camera.open(0);
    //Camera2.open(1);
    newCameraMatrix = cv::getOptimalNewCameraMatrix(
      cameraMatrix,
      distCoeffs,
      cv::Size(1920, 1080),
      1.0
  );
    if (Camera.isOpened())
    {
        UE_LOG(LogTemp, Warning, TEXT("Camera is Opened"));
        
        Camera.set(cv::CAP_PROP_FRAME_WIDTH, 1920); //My Webcam defaults it as 1270
        Camera.set(cv::CAP_PROP_FRAME_HEIGHT, 1080); //720
        Camera.set(cv::CAP_PROP_FPS, 60); //My webcam defaulted to 30 fps
       
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Camera is Not Opened"));
    }
    
    //if (Camera2.isOpened())
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Camera is Opened"));
    //}
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Camera is Not Opened"));
    //}
    
}

// Called every frame
void AOpenCVManager::Tick(float DeltaTime)
{
    double currentTime =
    (double)cv::getTickCount()
    / cv::getTickFrequency();
    
    
     float deltaTime = currentTime - PrevTime;
  //  Camera >> Frame;
    Camera2 >> Frame2;
    
    
    
    if (Camera.read(Frame))
    {
    cv::Mat UndistortedFrame;
    
        cv::undistort(
       Frame,
       UndistortedFrame,
       cameraMatrix,
       distCoeffs,
       newCameraMatrix
   );
        cv::Point2f center;
        float radius;
        
        if (DetectBall(UndistortedFrame, center, radius))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Camera1 ball detected at: %f, %f | Radius: %f"), center.x, center.y, radius);
            FVector3d camPos = calculate3dPosition(center, radius);
            FVector worldPos = OpenCVToUnreal(camPos);
            UE_LOG(LogTemp, Warning,
            TEXT("Camera1 ball REAL Position at: %f, %f, %f "), camPos.X,camPos.Y,camPos.Z);
            if(bHasPrevious3DPosition)
            {
                speed = calculateSpeed(
                    camPos,
                    previousPosition,
                    deltaTime
                );

                UE_LOG(LogTemp, Warning,
                    TEXT("3D Speed: %.2f cm/s"),
                    speed);
                
            }
           // trackBallSpeed(center, radius,22.8f);
        previousPosition = camPos;
            bHasPrevious3DPosition = true;
       
        }
    }
    
    if (Camera2.read(Frame2))
    {
        cv::Point2f center;
        float radius;
        
        if (DetectBall(Frame2, center, radius))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Camera2 ball detected at: %f, %f | Radius: %f"), center.x, center.y, radius);
        }
        
        
    }
    
    //Old Log data from getting OpenCV to init just test stuff DELETE LATER
    //UE_LOG(LogTemp, Warning,
    //TEXT("Frame: %d x %d & %f fps"),
    //Frame.cols, Frame.rows, Camera.get(cv::CAP_PROP_FPS));
    PrevTime = currentTime;
}

bool AOpenCVManager::DetectBall(cv::Mat& input, cv::Point2f& outCenter, float& outRadius)
{
    //Noise Reduction
    cv::Mat blur;
    cv::GaussianBlur(input, blur, cv::Size(9, 9), 2);

    //HSV apparently is better for colour detection
    cv::Mat HSV;
    cv::cvtColor(blur, HSV, cv::COLOR_BGR2HSV);

    //Range of colours to detect also mask
    cv::Scalar lower(20, 100, 100);
    cv::Scalar upper(35, 255, 255);

    cv::Mat mask;
    cv::inRange(HSV, lower, upper, mask);

    //close broken blobs in the frame
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1, -1), 2);

    //Find objects
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    //Find best shape
    if (!contours.empty())
    {
        int bestIndex = -1;
        double bestScore = 0;

        for (int i = 0; i < contours.size(); i++)
        {
            double area = cv::contourArea(contours[i]);

            if (area < 500)
            {
                continue;
            }

            double perimeter = cv::arcLength(contours[i], true);

            if (perimeter <= 0.0)
            {
                continue;
            }

            double circularity = 4.0 * CV_PI * area / (perimeter * perimeter);

            double score = area * circularity;

            if (score > bestScore)
            {
                bestScore = score;
                bestIndex = i;
            }
        }

        if (bestIndex == -1)
        {
            return false;
        }

        cv::minEnclosingCircle(contours[bestIndex], outCenter, outRadius);

        return true;
    }

    return false;
}

void AOpenCVManager::trackBallSpeed(cv::Point2f centre, float radius, float realBallDiameterCentimeter)
{

        
    float pixelDiameter = radius * 2.0f;
    float CentimetersPerPixel = realBallDiameterCentimeter / pixelDiameter;
    
    double currentTime = cv::getTickCount() / cv::getTickFrequency();

    if (previous_cx >= 0 && previous_cy >= 0)
    {
        // get change in position
        float dx = centre.x - previous_cx;
        float dy = centre.y - previous_cy;
        
        // euclidean distance formula 2d 
        float pixelDistance = sqrt((dx * dx) + (dy * dy));
        
        if (pixelDistance <= 1.0f)
        {
            speed = 0.0f; 
            return;
        }
        
        // get how many pixel is one cm
        float distanceCentimeters = pixelDistance * CentimetersPerPixel;
        float elapsedTime = currentTime - PrevTime;
        
        if (elapsedTime > 0.0f)
        {
            // v = d/t
            speed = distanceCentimeters / elapsedTime;
            UE_LOG(LogTemp, Warning,TEXT("Speed: %.2f cm/s | Diameter(px): %.2f"),speed, pixelDiameter);

               
        }
    }
    
    previous_cx = centre.x;
    previous_cy = centre.y; 
    PrevTime = currentTime;
}

float AOpenCVManager::calculateSpeed(FVector3d currentPos, FVector3d prevPos, float DeltaTime)
{
    float dx = currentPos.X - prevPos.X;
    float dy = currentPos.Y - prevPos.Y;
    float dz = currentPos.Z - prevPos.Z;

    float distance =
        sqrt(dx*dx + dy*dy + dz*dz);

    speed = distance /  DeltaTime;
    
    return speed; 
}

float AOpenCVManager::DistanceFromCamera(cv::Point2f centre, float radius)
{
    float focalLength = 1400.0f;

    float pixelDiameter = radius * 2.0f;

    float distanceCM =
        (focalLength * REALBALLDIAMETERCENTIMETER) // 22.8 is real ball diameter in cms
        / pixelDiameter;
    
    return distanceCM; 
}

FVector3d AOpenCVManager::calculate3dPosition(cv::Point2f centre, float radius)
{
    float fx = newCameraMatrix.at<double>(0,0);
    float fy = newCameraMatrix.at<double>(1,1);
    float cx = newCameraMatrix.at<double>(0,2);
    float cy = newCameraMatrix.at<double>(1,2);

    float pixelDiameter = radius * 2.0f;

    float Z =
        (fx * REALBALLDIAMETERCENTIMETER)
        / pixelDiameter;

    float X =
        ((centre.x - cx) * Z) / (fx);

    float Y =
        ((centre.y - cy) * Z) / (fy);
    
    return FVector3d(X, Y, Z);  
 
}
FVector AOpenCVManager::OpenCVToUnreal(const FVector3d& camPos)
{
    return FVector(
        camPos.Z,   // OpenCV Z → Unreal X (forward)
        camPos.X,   // OpenCV X → Unreal Y (right)
        -camPos.Y   // OpenCV Y → Unreal Z (up, flipped)
    );
}
void AOpenCVManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Camera.isOpened())
    {
        Camera.release();
        UE_LOG(LogTemp, Warning, TEXT("Camera released"));
    }

    Super::EndPlay(EndPlayReason);
}


//this function finds the position of the center of the ball in irl camera space
//axes: x is right, y is up, z is the direction the camera is looking
//to convert to unreal coords, swizzle xyz to yzx 
FVector3d AOpenCVManager::GetIRLCameraSpacePosition(FVector2d ball_center_position_in_screen_space, float ball_diameter_in_pixels,
    								                FVector2d camera_resolution, float camera_fov_along_x) {

    float ball_irl_radius = REALBALLDIAMETERCENTIMETER/2.0f;

    float ball_size_angle_percent = ball_diameter_in_pixels/camera_resolution.X;
    float ball_size_angle = ball_size_angle_percent*camera_fov_along_x;
    float ball_dist = ball_irl_radius/sin(ball_size_angle/2.0f);

    FVector2d ball_pixel_pos_relative_to_center = ball_center_position_in_screen_space-(camera_resolution/2.0f);

    float ball_pos_angle_percent = ball_pixel_pos_relative_to_center.Length()/camera_resolution.X;
    float ball_pos_angle = ball_pos_angle_percent*camera_fov_along_x;
    float ball_z_dist = ball_dist/sin(ball_pos_angle);

    float ball_x_pos_angle_percent = ball_pixel_pos_relative_to_center.X/camera_resolution.X;
    float ball_x_pos_angle = ball_x_pos_angle_percent*camera_fov_along_x;
    float ball_x_dist = ball_dist/cos(ball_x_pos_angle);

    float camera_fov_along_y = camera_fov_along_x*camera_resolution.Y/camera_resolution.X;
    float ball_y_pos_angle_percent = ball_pixel_pos_relative_to_center.Y/camera_resolution.Y;
    float ball_y_pos_angle = ball_y_pos_angle_percent*camera_fov_along_y;
    float ball_y_dist = ball_dist/cos(ball_y_pos_angle);

    FVector3d ball_irl_camera_space_pos = FVector3d(ball_x_dist, -ball_y_dist, ball_z_dist);
    
    return ball_irl_camera_space_pos;
}

FVector3d AOpenCVManager::GetInitialVelocityFromDataset(std::vector<float> timestamps, std::vector<FVector3d> positions) {

    if (timestamps.size() != positions.size()) {
        UE_LOG(LogTemp, Warning, TEXT("timestamp and position vectors passed into GetInitialVelocityFromDataset() do not have the same size"));
    }

    /*
    find parabola of best fit using least squares
    for each pair of (t, xyz), find:
    - sum(xyz)
    - sum(t*xyz)
    - sum(t^2*xyz)
    - sum(t)
    - sum(t^2)
    - sum(t^3)
    - sum(t^4)
    - number of pairs (n)
    */
    float x, tx, t2x,
          y, ty, t2y,
          z, tz, t2z,
          t, t2, t3, t4,
          n = std::min(timestamps.size(), positions.size());
    for (int i = 0; i < n; i++) {
        x   += positions[i].X;
        tx  += timestamps[i] * positions[i].X;
        t2x += timestamps[i] * timestamps[i] * positions[i].X;
        y   += positions[i].Y;
        ty  += timestamps[i] * positions[i].Y;
        t2y += timestamps[i] * timestamps[i] * positions[i].Y;
        z   += positions[i].Z;
        tz  += timestamps[i] * positions[i].Z;
        t2z += timestamps[i] * timestamps[i] * positions[i].Z;
        t   += timestamps[i];
        t2  += timestamps[i] * timestamps[i];
        t3  += timestamps[i] * timestamps[i] * timestamps[i];
        t4  += timestamps[i] * timestamps[i] * timestamps[i] * timestamps[i];
    }

    /*
    plug those coefficients into a 3x4 matrix
    [sum(t^4) sum(t^3) sum(t^2) sum(t^2*xyz)]
    [sum(t^3) sum(t^2) sum(t)   sum(t*xyz)  ] 
    [sum(t^2) sum(t)   n        sum(xyz)    ]
    
    turn that matrix into reduced row echelon form
    should look something like this
    [1 0 0 a]
    [0 1 0 b]
    [0 0 1 c]
    ideally you'd just do it once, but unreal doesn't have a 3x6 matrix or 6d vector class
    instead i'm just doing it for x, y, and z separately
    */
    FMatrix3x4 mx{
        t4, t3, t2, t2x,
        t3, t2, t , tx ,
        t2, t , n , x  };
    FMatrix3x4 my{
        t4, t3, t2, t2y,
        t3, t2, t , ty ,
        t2, t , n , y  };
    FMatrix3x4 mz{
        t4, t3, t2, t2z,
        t3, t2, t , tz ,
        t2, t , n , z  };

    mx = ConvertToRREF(mx);
    my = ConvertToRREF(my);
    mz = ConvertToRREF(mz);

    //plug resulting values into pos = a*t^2 + b*t + c to get the quadratic line of best fit
    //plug the first timestamp into this line to get the initial velocity
    FVector3d a{mx.M[0][3], my.M[0][3], mz.M[0][3]};
    FVector3d b{mx.M[1][3], my.M[1][3], mz.M[1][3]};
    FVector3d c{mx.M[2][3], my.M[2][3], mz.M[2][3]};
    float t0 = timestamps[0];
    FVector3d v0 = a*t0*t0 + b*t0 + c;

    return v0;
}

FMatrix3x4 AOpenCVManager::ConvertToRREF(FMatrix3x4 m) {

    FVector4d r0 = FVector4d(m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3]);
    FVector4d r1 = FVector4d(m.M[1][0], m.M[1][1], m.M[2][2], m.M[1][3]);
    FVector4d r2 = FVector4d(m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3]);

    r0 /= r0.X;
    r1 -= r1.X*r0;
    r2 -= r2.X*r0;
    r1 /= r1.Y;
    r2 -= r2.Y*r1;
    r0 -= r0.Y*r1;
    r2 /= r2.Z;
    r0 -= r0.Z*r2;
    r1 -= r1.Z*r2;

    FMatrix3x4 m2{};
    // ugh
    FMemory::Memcpy(&m2.M[0], &r0, sizeof(float) * 4);
    FMemory::Memcpy(&m2.M[1], &r1, sizeof(float) * 4);
    FMemory::Memcpy(&m2.M[2], &r2, sizeof(float) * 4);

    return m2;
}
