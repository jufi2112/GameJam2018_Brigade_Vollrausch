// Fill out your copyright notice in the Description page of Project Settings.

#include "HovercraftAIController.h"
#include "HoverTestGameModeBase.h"
#include "Engine/World.h"
#include "Hovercraft.h"

void AHovercraftAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AHoverTestGameModeBase* GameMode = Cast<AHoverTestGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not cast game mode in %s"), *GetName());
		return;
	}
	else
	{
		AHovercraft* Craft = Cast<AHovercraft>(GetPawn());
		if (!Craft)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not cast pawn in %s"), *GetName());
			return;
		}
		else
		{
			AActor* TargetActor = GameMode->GetAINextCheckpointActor(Craft->GetIndexOfLastCheckpoint());
			if (!TargetActor) { return; }
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("TargetActor: %s"), *TargetActor->GetName());
				//MoveToActor(TargetActor, AcceptanceRadius, false);



				FVector HovercraftForward = Craft->GetActorForwardVector();

				FVector AIForwardIntention = (TargetActor->GetActorLocation()-Craft->GetActorLocation()).GetSafeNormal();

				float ForwardThrow = FVector::DotProduct(HovercraftForward, AIForwardIntention);
				Craft->MoveForward(ForwardThrow);

				float RightThrow = FVector::CrossProduct(HovercraftForward, AIForwardIntention).Z;
				Craft->RotateRight(RightThrow);

				Craft->MoveRight(RightThrow);
			}
		}
	}
}




