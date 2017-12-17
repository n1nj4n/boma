// Fill out your copyright notice in the Description page of Project Settings.

#include "BMPlayer.h"
#include "components/InputComponent.h"
#include "components/SphereComponent.h"
#include "components/StaticMeshComponent.h"
#include "playfield.h"
// Sets default values
#pragma optimize("",off)
ABMPlayer::ABMPlayer()
{

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Collision->SetSphereRadius(49.9f,false);

	SetRootComponent(Collision);
	Mesh->SetupAttachment(Collision);
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Bombs=1;
	BombStrength=1;
	SpeedFactor=1;
	Speed=100.f;

	up=0;
	right=0;
	bTriggBomb=false;

}

// Called when the game starts or when spawned
void ABMPlayer::BeginPlay()
{
	Super::BeginPlay();
	Rotation=GetActorRotation();
}

// Called to bind functionality to input
void ABMPlayer::SetupPlayerInputComponent(UInputComponent* ic)
{
	Super::SetupPlayerInputComponent(ic);
//	ic->BindAction("Human1Fire",IE_Released,this,&ABMPlayer::Fire);
//	ic->BindAxis("Human1Up",this,&ABMPlayer::Up);
//	ic->BindAxis("Human1Right",this,&ABMPlayer::Right);

}

void ABMPlayer::Fire()
{
	if(up || right)
	{
		bTriggBomb=true;
		return;
	}
	if(BombTemplate)
	{
		MainPawn->SpawnTemplate(BombTemplate,GetActorLocation(),this);
	}
}
void ABMPlayer::Up(float amount)
{
	float a=FMath::Abs(amount);
	if(a < 0.1f)
		return;
	if(right!=0 || up!=0)
		return;
	FVector dir(0,0,0);
	FRotator oldRot=Rotation;
	if(amount>0){up=100;Rotation.Yaw=0;dir.X=100.f;}
	else {up=-100;Rotation.Yaw=180;dir.X=-100.f;}
	AddActorLocalRotation(Rotation-oldRot);
	FVector pos=GetActorLocation();
	if(MainPawn)
	{
		if(MainPawn->CheckUnbreakable(pos+dir))
		{
			up=0;
		}
		if(up!=0)
		{

			if(MainPawn->CheckBreakable(pos+dir)>=0)
			{
				up=0;
			}
		}
	}


}
void ABMPlayer::Right(float amount)
{
	float a=FMath::Abs(amount);
	if(a < 0.1f)
		return;
	if(right!=0 || up!=0)
		return;
	FVector dir(0,0,0);
	FRotator oldRot=Rotation;
	if(amount>0){right=100;Rotation.Yaw=90;dir.Y=100.f;}
	else {right=-100;Rotation.Yaw=-90;dir.Y=-100.f;}
	AddActorLocalRotation(Rotation-oldRot);
	if(MainPawn)
	{
		FVector pos=GetActorLocation();
		if(MainPawn->CheckUnbreakable(pos+dir))
		{
			right=0;
		}
		if(right!=0)
		{
			if(MainPawn->CheckBreakable(pos+dir)>=0)
			{
				right=0;
			}
		}
	}
}
void ABMPlayer::Move(float step)
{
	FVector stepDir(0,0,0);


	if(up!=0)
	{
		if(up>step)
		{
			up-=step;
			stepDir=FVector(step,0,0);
		}
		else if(up<-step)
		{
			up+=step;
			stepDir=FVector(-step,0,0);
		}
		else
		{
			stepDir=FVector(up,0,0);
			up=0;
		}

	}
	else if(right!=0)
	{
		if(right>step)
		{
			right-=step;
			stepDir=FVector(0,step,0);	
		}
		else if(right<-step)
		{
			right+=step;
			stepDir=FVector(0,-step,0);	
		}
		else
		{
			stepDir=FVector(0,right,0);	
			right=0;
		}

	}
	AddActorWorldOffset(stepDir,true);
//	r+=rotDir;
}

// Called every frame
void ABMPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Move(Speed*SpeedFactor*DeltaTime);
	if(bTriggBomb)
	{
		if(right==0 && up==0)
		{
			bTriggBomb=false;
			Fire();
		}
	}
}
class APlayfield* ABMPlayer::GetPlayfield()
{
	return MainPawn;
}
void ABMPlayer::KillYourself()
{
	MainPawn->RemovePlayer(this);
}

#pragma optimize("",on)
