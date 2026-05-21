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
private:
	//Our Two Cams
	cv::VideoCapture Camera;
	cv::VideoCapture Camera2;
	//2 Frames for the two cams
	cv::Mat Frame;
	cv::Mat Frame2;
	
	

	bool DetectBall(cv::Mat& input, cv::Point2f& outCenter, float& outRadius);
	
	
	int previous_cx = -1;
	int previous_cy =  -1;
	double PrevTime = 0.0;
	float speed = 0.0f;
	const float REALBALLDIAMETERCENTIMETER = 6.0f;
	
	cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) <<
	1400, 0, 960,
	0, 1400, 540,
	0, 0, 1);
	
	cv::Mat distCoeffs =
	cv::Mat::zeros(1,5,CV_64F);
	
	cv::Mat newCameraMatrix;
	
	FVector3d previousPosition;
	bool bHasPrevious3DPosition = false;
	
	void trackBallSpeed(cv::Point2f centre,float radius, float realBallDiameterCentimeter); 
	float calculateSpeed(FVector3d currentPos, FVector3d prevPos,float DeltaTime); 
	float DistanceFromCamera(cv::Point2f centre,float radius);
	FVector3d calculate3dPosition(cv::Point2f centre,float radius);
	FVector OpenCVToUnreal(const FVector3d& camPos);
	
	//LATER BRINGS
	//Speed Vector
};