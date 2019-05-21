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
			// notify game mode that transition is finished
			if (GetWorld())
			{
				AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(GetWorld()->GetAuthGameMode());
				if (GameMode)
				{
					bTransitionInProcess = false;
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
	else
	{
		// check if multipoint transition is in process
		if (bIsMultipointTransitionInProcess)
		{
			// check if we are at transition from start point to intermediate point at spawn elevation
			if (!bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation)
			{
				FVector NewPosition = FMath::VInterpTo(GetActorLocation(), MultipointTransitionIntermediatePointAtStartCoordinates, DeltaTime, MultipointTransitionSpeed);

				if (FVector::Distance(GetActorLocation(), NewPosition) <= MultipointTransitionDeltaToStop)
				{
					SetActorLocation(NewPosition);
					bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation = true;
					// just in case
					bHasMultipointTransitionReachedTargetPointAtSpawnElevation = false;
				}
				else
				{
					SetActorLocation(NewPosition);
				}
			}
			// check if we are at transition from intermediate point at spawn elevation to target end point at spawn elevation
			else if (!bHasMultipointTransitionReachedTargetPointAtSpawnElevation)
			{
				FVector NewPosition = FMath::VInterpTo(GetActorLocation(), MultipointTransitionIntermediatePointAtFinalCoordinates, DeltaTime, MultipointTransitionSpeed);

				if (FVector::Distance(GetActorLocation(), NewPosition) <= MultipointTransitionDeltaToStop)
				{
					SetActorLocation(NewPosition);
					bHasMultipointTransitionReachedTargetPointAtSpawnElevation = true;
					// just in case
					bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation = true;
				}
				else
				{
					SetActorLocation(NewPosition);
				}
			}
			else if (bHasMultipointTransitionReachedTargetPointAtSpawnElevation && bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation)
			{
				// we are at transition to target end point

				// check if we are allowed to zoom to end point
				if (bIsMultipointTransitionAllowedToZoomToEndPoint)
				{
					FVector NewPosition = FMath::VInterpTo(GetActorLocation(), MultipointTransitionTargetEndPoint, DeltaTime, MultipointTransitionSpeed);

					if (FVector::Distance(GetActorLocation(), NewPosition) <= MultipointTransitionDeltaToStop)
					{
						// notify game mode that multipoint transition is finished
						UWorld* World = GetWorld();
						if (World)
						{
							AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(World->GetAuthGameMode());
							if (GameMode)
							{
								// we reached the target end point, so reset all variables used
								bIsMultipointTransitionInProcess = false;
								bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation = false;
								bHasMultipointTransitionReachedTargetPointAtSpawnElevation = false;
								bIsMultipointTransitionAllowedToZoomToEndPoint = false;
								MultipointTransitionIntermediatePointAtStartCoordinates = FVector();
								MultipointTransitionTargetEndPoint = FVector();
								MultipointTransitionIntermediatePointAtFinalCoordinates = FVector();

								// notify game mode
								GameMode->DefaultPawnFinishedMultipointTransition();
								// reset transform
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

EControllerType AProceduralDefaultPawn::GetControllerType() const
{
	return ControllerType;
}

void AProceduralDefaultPawn::StartMultipointTransition(const FVector TargetStartLocation, const FVector TargetEndLocation, const float Speed, const float DeltaToStop)
{
	// first set current location to TargetStartLocation
	SetActorLocation(TargetStartLocation);
	MultipointTransitionTargetEndPoint = TargetEndLocation;
	MultipointTransitionIntermediatePointAtStartCoordinates = FVector(TargetStartLocation.X, TargetStartLocation.Y, SpawnTransform.GetLocation().Z);
	MultipointTransitionIntermediatePointAtFinalCoordinates = FVector(TargetEndLocation.X, TargetEndLocation.Y, SpawnTransform.GetLocation().Z);
	bIsMultipointTransitionInProcess = true;
	bHasMultipointTransitionReachedIntermediatePointAtSpawnElevation = false;
	bHasMultipointTransitionReachedTargetPointAtSpawnElevation = false;
	// allow this if all tiles are loaded around needed location
	bIsMultipointTransitionAllowedToZoomToEndPoint = false;
	MultipointTransitionSpeed = Speed;
	MultipointTransitionDeltaToStop = DeltaToStop;
}

void AProceduralDefaultPawn::AllowMultipointTransitionZoomToEndPoint()
{
	bIsMultipointTransitionAllowedToZoomToEndPoint = true;
}

