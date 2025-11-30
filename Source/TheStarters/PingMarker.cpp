// Fill out your copyright notice in the Description page of Project Settings.


#include "PingMarker.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.h"

APingMarker::APingMarker()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = false;
}

void APingMarker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APingMarker, TeamID);
}

bool APingMarker::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // 1. If it's the server, it's always relevant (so it exists)
    if (HasAuthority()) return true;

    // 2. RealViewer is usually the PlayerController. We need to get the Pawn.
    const APlayerController* PC = Cast<APlayerController>(RealViewer);
    if (!PC) return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

    const ABaseCharacter* ViewerPawn = Cast<ABaseCharacter>(PC->GetPawn());

    // 3. Compare Team IDs
    if (ViewerPawn)
    {
        // If the viewer has the same TeamID as the Ping, replicate it!
        return ViewerPawn->TeamID == this->TeamID;
    }

    return false;
}

