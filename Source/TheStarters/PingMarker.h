// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PingMarker.generated.h"

UCLASS()
class THESTARTERS_API APingMarker : public AActor
{
	GENERATED_BODY()
	
public:
    APingMarker();

    // The Team ID this ping belongs to
    UPROPERTY(Replicated)
    int32 TeamID = -1;

    // Override this to filter visibility
    virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
