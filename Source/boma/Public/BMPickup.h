// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BMPickup.generated.h"

UCLASS()
class BOMA_API ABMPickup : public AActor
{
	GENERATED_BODY()

	// collision
	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (AllowPrivateAccess = "true"))
	class USphereComponent*	Collision;

	// Rendered mesh
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent*	PickupMesh;

	// Override material of PickupMesh
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface*	PickupMaterial;
	
	// rotation speed per second in degrees
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float	RotationSpeed;

	// time of creation
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float	createTime;

public:	
	// Sets default values for this actor's properties
	ABMPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:	
	
};
