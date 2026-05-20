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
    Super::Tick(DeltaTime);
    Camera >> Frame;
    Camera2 >> Frame2;
    
    if (Camera.read(Frame))
    {
        cv::Point2f center;
        float radius;
        
        if (DetectBall(Frame, center, radius))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Camera1 ball detected at: %f, %f | Radius: %f"), center.x, center.y, radius);
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

void AOpenCVManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Camera.isOpened())
    {
        Camera.release();
        UE_LOG(LogTemp, Warning, TEXT("Camera released"));
    }

    Super::EndPlay(EndPlayReason);
}

