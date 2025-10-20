// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "OnlineSessionSettings.h"

#include "EOS_GameInstance.generated.h"
/**
 * 
 */

UCLASS()
class THESTARTERS_API UEOS_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	void LoginWithEOS(FString ID, FString Token, FString LoginType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Functions")
	FString GetPlayerUsername();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Functions")
	bool IsPlayerLoggedIn();

	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	void CreateEOSSession(bool isDedicatedServer, bool isLanServer, int32 numOfpublicConnections);

	/*UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	void FindSessionAndJoin();

	UFUNCTION(BlueprintCallable, Category = "EOS Functions")
	void JoinSession();*/

	//TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOS Functions")
	FString OpenLevelText;



	void OnLoginEOSCompleted(int32 LocalUserNum, bool Success, const FUniqueNetId& UserId, const FString& Error);
	void OnCreateSessionCompleted(FName SessionName, bool isSuccesful);
	void OnFindSessionCompleted(bool isSuccesful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
