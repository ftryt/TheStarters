// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputActionValue.h" // обовТ€зково
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "BaseCharacter.generated.h"

UCLASS(Abstract) // робить клас абстрактним (не можна створити напр€му)
class THESTARTERS_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Enhanced Input Actions
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputMappingContext* PlayerMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* SprintAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* SpecialAbilityAction;

    // Input functions
    void Move(const FInputActionValue& Value);
    void StartSprint(const FInputActionValue& Value);
    void StopSprint(const FInputActionValue& Value);
    void SpecialAbility(const FInputActionValue& Value);

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SprintMultiplier = 1.5f;

    bool bIsSprinting = false;
};
