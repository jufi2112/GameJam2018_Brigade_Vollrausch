// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralCheckpoint.h"
#include "Classes/Components/ShapeComponent.h"
#include "Hovercraft.h"
#include "HovercraftPlayerController.h"
#include "HoverTestGameModeProceduralLevel.h"


// Sets default values
AProceduralCheckpoint::AProceduralCheckpoint()
{
 	// checkpoint don't need to tick
	PrimaryActorTick.bCanEverTick = false;

	Tags.Add(FName("ProceduralCheckpoint"));

}

// Called when the game starts or when spawned
void AProceduralCheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProceduralCheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralCheckpoint::SetCheckpointID(const uint32 NewCheckpointID)
{
	CheckpointID = NewCheckpointID;
}

uint32 AProceduralCheckpoint::GetCheckpointID() const
{
	return CheckpointID;
}

void AProceduralCheckpoint::SetCollisionShapeReference(UShapeComponent * ReferenceToSet)
{
	if (!ReferenceToSet)
	{
		UE_LOG(LogTemp, Error, TEXT("Provided reference in SetCollisionShapeReference is null pointer in %s"), *GetName());
		return;
	}

	CollisionShape = ReferenceToSet;

	if (CollisionShape)
	{
		CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &AProceduralCheckpoint::OnBeginOverlap);
	}
}

void AProceduralCheckpoint::OnBeginOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor)
	{
		AHovercraft* Hovercraft = Cast<AHovercraft>(OtherActor);

		if (Hovercraft)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(Hovercraft->GetController());
			if (PC)
			{
				AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(GetWorld()->GetAuthGameMode());
				if (!GameMode)
				{
					UE_LOG(LogTemp, Error, TEXT("Could not retrieve game mode in OnBeginOverlap in %s"), *GetName());
					return;
				}
				else
				{
					GameMode->HandlePlayerHovercraftCheckpointOverlap(Hovercraft, PC, this);
				}
			}
			else
			{
				// add AI stuff here
			}
		}
	}
}

