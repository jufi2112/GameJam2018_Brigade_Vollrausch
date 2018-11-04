// Fill out your copyright notice in the Description page of Project Settings.

#include "FinishLine.h"
#include "Hovercraft.h"
#include "HoverTestGameModeBase.h"
#include "Engine/World.h"
#include "Engine/Classes/Components/ShapeComponent.h"


// Sets default values
AFinishLine::AFinishLine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFinishLine::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFinishLine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFinishLine::OnBeginOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor)
	{
		AHovercraft* Craft = Cast<AHovercraft>(OtherActor);
		if (Craft)
		{
			// call game mode function
			AHoverTestGameModeBase* GameMode = Cast<AHoverTestGameModeBase>(GetWorld()->GetAuthGameMode());
			if (!GameMode)
			{
				UE_LOG(LogTemp, Error, TEXT("Could not cast game mode in %s"), *GetName());
				return;
			}
			else
			{
				GameMode->HandleHovercraftFinishLineOverlap(Craft, this);
			}
		}
	}
}

void AFinishLine::SetCollisionShapeReference(UShapeComponent * ReferenceToSet)
{
	CollisionShape = ReferenceToSet;
	
	if (CollisionShape)
	{
		CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &AFinishLine::OnBeginOverlap);
	}
}

