// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EOS_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THESTARTERS_API AEOS_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	// Does not work as intended, is never called
	virtual void OnNetCleanup(UNetConnection* Connection) override;
};
