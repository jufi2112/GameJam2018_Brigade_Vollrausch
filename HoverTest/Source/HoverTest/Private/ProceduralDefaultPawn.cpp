// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralDefaultPawn.h"
#include "HoverTestGameModeProceduralLevel.h"
#include "Engine/World.h"


// Sets default values
AProceduralDefaultPawn::AProceduralDefaultPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AProceduralDefaultPawn::BeginPlay()
{
	Super::BeginPlay();

	SpawnTransform = GetActorTransform();
	
}

// Called every frame
void AProceduralDefaultPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// check if transition is in process
	if (bTransitionInProcess)
	{
		FVector NewPosition = FMath::VInterpTo(GetActorLocation(), TransitionTarget, DeltaTime, TransitionSpeed);

		if (FVector::Distance(GetActorLocation(), NewPosition) <= TransitionDeltaToStop)
		{
			bTransitionInProcess = false;
			// notify game mode that transition is finished
			if (GetWorld())
			{
				AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(GetWorld()->GetAuthGameMode());
				if (GameMode)
				{
					GameMode->DefaultPawnFinishedTransition();
					SetActorTransform(SpawnTransform);
				}
			}
		}
		else
		{
			SetActorLocation(NewPosition);
		}
	}

}

// Called to bind functionality to input
void AProceduralDefaultPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AProceduralDefaultPawn::StartTransition(const FVector Target, const float Speed, const float DeltaToStop)
{
	if (!bTransitionInProcess)
	{
		bTransitionInProcess = true;
		TransitionTarget = Target;
		TransitionSpeed = Speed;
		TransitionDeltaToStop = DeltaToStop;
	}
}

