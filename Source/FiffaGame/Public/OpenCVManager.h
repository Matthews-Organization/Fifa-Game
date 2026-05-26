// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "PreOpenCVHeaders.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "PostOpenCVHeaders.h"
#include <opencv2/opencv.hpp>
#include "Math/Matrix.h"
#include "Matrix3x4.h"

#include "OpenCVManager.generated.h"

UCLASS()

class FIFFAGAME_API AOpenCVManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpenCVManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(BlueprintReadOnly, Category="OpenCV")
	UTexture2D* MaskTexture;
	
	UFUNCTION(BlueprintCallable, Category="OpenCV")
	UTexture2D* GetMaskTexture();
private:
	//Our Two Cams
	cv::VideoCapture Camera;
	cv::VideoCapture Camera2;
	//2 Frames for the two cams
	cv::Mat Frame;
	cv::Mat Frame2;
	
	

	bool DetectBall(cv::Mat& input, cv::Point2f& outCenter, float& outRadius);
	UTexture2D* CreateTextureFromMat(const cv::Mat& Mat);
	
	int previous_cx = -1;
	int previous_cy =  -1;
	double PrevTime = 0.0;
	float speed = 0.0f;
	const float REALBALLDIAMETERCENTIMETER = 17.8f;
	
	cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) <<
	1400, 0, 960,
	0, 1400, 540,
	0, 0, 1);
	
	cv::Mat distCoeffs =
	cv::Mat::zeros(1,5,CV_64F);
	
	cv::Mat newCameraMatrix;
	
	FVector3d previousPosition;
	bool bHasPrevious3DPosition = false;

	std::array<float, 20> tracked_timestamps;
	std::array<FVector3d, 20> tracked_positions;
	
	void trackBallSpeed(cv::Point2f centre,float radius, float realBallDiameterCentimeter); 
	float calculateSpeed(FVector3d currentPos, FVector3d prevPos,float DeltaTime); 
	float DistanceFromCamera(cv::Point2f centre,float radius);
	FVector3d calculate3dPosition(cv::Point2f centre,float radius);
	FVector OpenCVToUnreal(const FVector3d& camPos);

	//this function finds the position of the center of the ball in irl camera space
	//axes: x is right, y is up, z is the direction the camera is looking
	//to convert to unreal coords, swizzle xyz to yzx 
	FVector3d GetIRLCameraSpacePosition(FVector2d ball_center_position_in_screen_space, float ball_diameter_in_pixels, 
										FVector2d camera_resolution, float camera_fov_along_x);
	//this function finds the initial velocity of the ball from a list of timestamps and a list of positions at those timestamps
	FVector3d GetInitialVelocityFromDataset(std::vector<float> timestamps, std::vector<FVector3d> positions);
	//converts a 3x4 matrix to reduced row echelon form.
	//helper function for GetInitialVelocityFromDataset()
	FMatrix3x4 ConvertToRREF(FMatrix3x4 m);

	//LATER BRINGS
	//Speed Vector
};