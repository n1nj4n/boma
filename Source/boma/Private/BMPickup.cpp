// Fill out your copyright notice in the Description page of Project Settings.

#include "BMPickup.h"
#include "components/SphereComponent.h"
#include "components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABMPickup::ABMPickup()
{
	// setup collision object
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetSphereRadius(49.9f,false);
	Collision->SetCollisionProfileName("BMPickup");

	// setup mesh component
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetCollisionProfileName("NoCollision");
	PickupMesh->SetupAttachment(Collision);

	PrimaryActorTick.bCanEverTick = true;

	// defaults
	RotationSpeed=90.f;

	createTime=0;

}

// Called when the game starts or when spawned
void ABMPickup::BeginPlay()
{
	Super::BeginPlay();
	if(PickupMaterial)
		PickupMesh->SetMaterial(0,PickupMaterial);

	// we need a creation time, not to destroy the object prematurely.
	createTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	
}

void ABMPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// apply local rotation to mesh
	float rad=FMath::DegreesToRadians(RotationSpeed*DeltaTime);
	PickupMesh->AddLocalRotation(FQuat(FVector(0,0,1),rad));

}

