// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#define SEARCH_KEYWORDS TEXT("SearchKeywords")

void UEOS_GameInstance::LoginWithEOS(FString ID, FString Token, FString LoginType)
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	IOnlineIdentityPtr IdentityInterface = SubsystemRef->GetIdentityInterface();

	if (IdentityInterface) {
		FOnlineAccountCredentials AccountDetails;

		AccountDetails.Id = ID;
		AccountDetails.Token = Token;
		AccountDetails.Type = LoginType;

		// Add the callback function to handle login completion
		IdentityInterface->OnLoginCompleteDelegates->AddUObject(this, &UEOS_GameInstance::OnLoginEOSCompleted);

		// Initiate login process
		IdentityInterface->Login(0, AccountDetails);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
	}
}

FString UEOS_GameInstance::GetPlayerUsername()
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
	FString userName = "Null";

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return userName;
	}

	IOnlineIdentityPtr IdentityInterface = SubsystemRef->GetIdentityInterface();

	if (!IdentityInterface) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
		return userName;
	}

	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn) {
		userName = IdentityInterface->GetPlayerNickname(0);
	}

	return userName;
}

bool UEOS_GameInstance::IsPlayerLoggedIn()
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
	bool isLogged = false;

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return isLogged;
	}

	IOnlineIdentityPtr IdentityInterface = SubsystemRef->GetIdentityInterface();

	if (!IdentityInterface) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
		return isLogged;
	}

	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn) {
		isLogged = true;
	}

	return isLogged;
}

void UEOS_GameInstance::CreateEOSSession(bool isDedicatedServer, bool isLanServer, int32 numOfpublicConnections)
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();

	if (!SessionPtrRef) {
		UE_LOG(LogTemp, Error, TEXT("GetSessionInterface FAIL"));
		return;
	}

	FOnlineSessionSettings sessionInfo;
	sessionInfo.bIsDedicated = isDedicatedServer;
	sessionInfo.bAllowInvites = true;
	sessionInfo.bIsLANMatch = isLanServer;
	sessionInfo.NumPublicConnections = numOfpublicConnections;
	sessionInfo.bUseLobbiesIfAvailable = true; //!!!
	sessionInfo.bUsesPresence = false;
	sessionInfo.bShouldAdvertise = true;
	sessionInfo.Set(SEARCH_KEYWORDS, FString("RandomHi"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnCreateSessionCompleted);
	SessionPtrRef->CreateSession(0, FName("MainSession"), sessionInfo);
}

void UEOS_GameInstance::OnLoginEOSCompleted(int32 LocalUserNum, bool Success, const FUniqueNetId& UserId, const FString& Error)
{
	if (Success) {
		UE_LOG(LogTemp, Warning, TEXT("Login Success - UserID: %s"), *UserId.ToString());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Login FAIL, Reason - %s"), *Error);
	}
}

void UEOS_GameInstance::OnCreateSessionCompleted(FName SessionName, bool isSuccesful)
{
	if (!isSuccesful) {
		UE_LOG(LogTemp, Error, TEXT("Cant create session: OnCreateSessionCompleted"));
		return;
	}

	GetWorld()->ServerTravel(OpenLevelText);
}

void UEOS_GameInstance::OnFindSessionCompleted(bool isSuccesful)
{
}

void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

}
