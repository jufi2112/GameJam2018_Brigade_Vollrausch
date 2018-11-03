// Fill out your copyright notice in the Description page of Project Settings.

#include "Checkpoint.h"
#include "Classes/Components/BoxComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "HovercraftPlayerController.h"
#include "Hovercraft.h"


// Sets default values
ACheckpoint::ACheckpoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	/*RootComponent = CreateDefaultSubobject<USceneComponent>(FName("Root Component"));
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("Collision Box"));
	if (!CollisionBox)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create BoxComponent in %s"), *GetName());
	}
	else
	{
		CollisionBox->SetupAttachment(RootComponent);
	}*/

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
		UE_LOG(LogTemp, Warning, TEXT("%s got overlapped by %s"), *GetName(), *OtherActor->GetName());
		AHovercraft* Craft = Cast<AHovercraft>(OtherActor);
		if (Craft)
		{
			AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(Craft->GetController());
			if (PC)
			{
				/* OtherActor is player controlled */
				PC->SetResetPosition(GetActorLocation());
				PC->SetResetYaw(HovercraftResetYaw);
			}
		}
		
	}
}

