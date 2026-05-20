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
	//LATER BRINGS
	//Speed Vector
};