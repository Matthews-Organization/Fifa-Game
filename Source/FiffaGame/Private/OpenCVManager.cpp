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
    
    if (Camera.read(Frame))
    {
        //Noise Reduction
        cv::GaussianBlur(Frame, Frame, cv::Size(9, 9), 2);

        //HSV apparently is better for colour detection
        cv::Mat HSV;
        cv::cvtColor(Frame, HSV, cv::COLOR_BGR2HSV);

        //Range of colours to detect
        cv::Scalar lower(20, 100, 100);
        cv::Scalar upper(35, 255, 255);

        cv::Mat mask;
        cv::inRange(HSV, lower, upper, mask);

        //Clean Noise? Eroding removes random noise, Dilating fills holes
        cv::erode(mask, mask, cv::Mat(), cv::Point(-1,-1), 2);
        cv::dilate(mask, mask, cv::Mat(), cv::Point(-1,-1), 2);

        //FIND SHAPES
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        if (!contours.empty())
        {
            //FIND LARGEST SHAPE
            int largestIndex = 0;
            double maxArea = 0;

            for (int i = 0; i < contours.size(); i++)
            {
                double area = cv::contourArea(contours[i]);
                if (area > maxArea)
                {
                    maxArea = area;
                    largestIndex = i;
                }
            }

            //Get Center of Shape
            cv::Moments m = cv::moments(contours[largestIndex]);

            if (m.m00 != 0)
            {
                int cx = static_cast<int>(m.m10 / m.m00);
                int cy = static_cast<int>(m.m01 / m.m00);
                
                //Write it out for now
                UE_LOG(LogTemp, Warning, TEXT("Ball at: %d, %d"), cx, cy);
            }
        }
    }
    
    //Old Log data from getting OpenCV to init just test stuff DELETE LATER
    //UE_LOG(LogTemp, Warning,
    //TEXT("Frame: %d x %d & %f fps"),
    //Frame.cols, Frame.rows, Camera.get(cv::CAP_PROP_FPS));
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

