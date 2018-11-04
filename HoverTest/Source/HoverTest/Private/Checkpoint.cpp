// Fill out your copyright notice in the Description page of Project Settings.

#include "Checkpoint.h"
#include "Classes/Components/BoxComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "HovercraftPlayerController.h"
#include "Hovercraft.h"
#include "HoverTestGameModeBase.h"
#include "Engine/World.h"
#include "HovercraftAIController.h"


// Sets default values
ACheckpoint::ACheckpoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();


	
}

// Called every frame
void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACheckpoint::SetCollisionShapeReference(UShapeComponent * ReferenceToSet)
{
	CollisionShape = ReferenceToSet;

	if (CollisionShape)
	{
		CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnBeginOverlap);
	}
}

void ACheckpoint::OnBeginOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor)
	{
		AHovercraft* Craft = Cast<AHovercraft>(OtherActor);
		if (Craft)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(Craft->GetController());
			if (PC)
			{
				/* OtherActor is player controlled */
				// call game mode function
				AHoverTestGameModeBase* GameMode = Cast<AHoverTestGameModeBase>(GetWorld()->GetAuthGameMode());
				if (!GameMode)
				{
					UE_LOG(LogTemp, Error, TEXT("Could not cast the game mode to the correct class in %s"), *GetName());
					return;
				}
				else
				{
					GameMode->HandlePlayerHovercraftCheckpointOverlap(Craft, PC, this);
				}
			}
			else
			{
				AHovercraftAIController* AIController = Cast<AHovercraftAIController>(Craft->GetController());
				if (AIController)
				{
					/* AI */
					AHoverTestGameModeBase* GameMode = Cast<AHoverTestGameModeBase>(GetWorld()->GetAuthGameMode());
					if (!GameMode)
					{
						UE_LOG(LogTemp, Error, TEXT("Could not cast the game mode to the correct class in %s"), *GetName());
						return;
					}
					else
					{
						GameMode->HandleAIHovercraftCheckpointOverlap(Craft, this);
					}
				}
			}
		}
		
	}
}

int32 ACheckpoint::GetCheckpointIndex() const
{
	return IndexOnTrack;
}

