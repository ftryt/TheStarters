// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("BaseCharacter: V1 Initialized!"));
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("BaseCharacter: V1 Initialized!"));

    // Додаємо Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(PlayerMappingContext, 0);
        }
    }

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ABaseCharacter::StartSprint);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);
        EnhancedInput->BindAction(SpecialAbilityAction, ETriggerEvent::Started, this, &ABaseCharacter::SpecialAbility);
    }
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ABaseCharacter::StartSprint(const FInputActionValue& Value)
{
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed * SprintMultiplier;
}

void ABaseCharacter::StopSprint(const FInputActionValue& Value)
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseCharacter::SpecialAbility(const FInputActionValue& Value)
{
    //UE_LOG(LogTemp, Log, TEXT("Special Ability triggered"));
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Special Ability triggered"));
}
