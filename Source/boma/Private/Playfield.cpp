// Fill out your copyright notice in the Description page of Project Settings.

#include "Playfield.h"
#include "bmplayer.h"
#include "bmpickup.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

#pragma optimize("",off)
// 22
// Sets default values
APlayfield::APlayfield()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Unbreakable = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Unbreakable"));
	Breakable = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Breakable"));
	SetRootComponent(Root);
	Camera->SetupAttachment(Root);
	Camera->SetRelativeLocationAndRotation(FVector(0,0,2000),FRotator(-90,0,0).Quaternion());
	Unbreakable->SetupAttachment(Root);
	Breakable->SetupAttachment(Root);

	//	UnbreakableMaterial=nullptr;
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TemplatePlayer1=nullptr;
	TemplatePlayer2=nullptr;
	TemplatePlayer3=nullptr;
	TemplatePlayer4=nullptr;
	XSize=15;
	YSize=15;
	initializeGame=false;
	memset(players,0,sizeof(players));
	tileAlloc=nullptr;

	NoLevelBuilding=false;

	PickupSpawnPercetage=30.f;
	gameTime=120.f;

}
void APlayfield::CreateMap()
{
	if(NoLevelBuilding)
		return;
	Unbreakable->ClearInstances();
	Breakable->ClearInstances();
	uint8* tileAlloc=new uint8[XSize*YSize];
	memset(tileAlloc,0,XSize*YSize);

	// allocate space for players to spawn
	// player 1
	tileAlloc[0]=1;
	tileAlloc[1]=1;
	tileAlloc[XSize]=1;

	// player 2
	tileAlloc[XSize-1]=1;
	tileAlloc[XSize-2]=1;
	tileAlloc[XSize*2-1]=1;

	// player 3
	int offset=XSize*(YSize-2);
	tileAlloc[offset]=1;
	tileAlloc[offset+XSize]=1;
	tileAlloc[offset+XSize+1]=1;

	// player 4
	offset+=XSize;
	tileAlloc[offset-1]=1;
	tileAlloc[offset+XSize-2]=1;
	tileAlloc[offset+XSize-1]=1;

	// randomize away some destructible walls.
	for(int i=0;i<XSize*YSize;i++)
	{
		if(FMath::RandRange(0,1)>0.5f)
			tileAlloc[i]=1;

	}


	// create surrounding wall
	FVector realSize=FVector(YSize,XSize,0)*100.f;
	FVector halfSize=realSize*0.5f;
	FTransform tr;
	FVector topleft(halfSize.X+100.f,-halfSize.Y,0);
	FVector botleft(-halfSize.X,-halfSize.Y,0);
	// left <-> right walls
	for(int x=0;x<XSize;x++)
	{
		tr.SetLocation(topleft);
		Unbreakable->AddInstance(tr);
		topleft.Y+=100.f;
		tr.SetLocation(botleft);
		Unbreakable->AddInstance(tr);
		botleft.Y+=100.f;
	}
	// up <-> down walls
	topleft=FVector(halfSize.X,-halfSize.Y-100.f,0);
	FVector topright(halfSize.X,halfSize.Y,0);
	for(int y=0;y<YSize;y++)
	{
		tr.SetLocation(topleft);
		Unbreakable->AddInstance(tr);
		topleft.X-=100.f;
		tr.SetLocation(topright);
		Unbreakable->AddInstance(tr);
		topright.X-=100.f;
	}
	// place indestructible walls on the play field
	topleft=FVector(halfSize.X,-halfSize.Y,0);
	for(int y=1;y<YSize-1;y++)
	{
		for(int x=1;x<XSize-1;x++)
		{
			if(x&1 && y&1)
			{
				tileAlloc[XSize*y+x]=1;
				FVector pos(-y,x,0);
				pos*=100.f;
				tr.SetLocation(topleft+pos);
				Unbreakable->AddInstance(tr);
			}
		}
	}
	// place destructible walls where there is nothing else allocated
	for(int y=0;y<YSize;y++)
	{
		for(int x=0;x<XSize;x++)
		{
			if(!tileAlloc[XSize*y+x])
			{
				FVector pos(-y,x,-0.15);
				pos*=100.f;
				tr.SetLocation(topleft+pos);
				Breakable->AddInstance(tr);
			}
		}
	}
	delete[] tileAlloc;
}

bool APlayfield::CheckUnbreakable(const FVector& center)
{
	TArray<int32> idx;
	idx=Unbreakable->GetInstancesOverlappingSphere(center, 10.5f);
	return idx.Num()!=0;
}
int32 APlayfield::CheckBreakable(const FVector& center)
{
	TArray<int32> idx;
	idx=Breakable->GetInstancesOverlappingSphere(center, 10.5f);
	return idx.Num()?idx[0]:-1;
}
bool APlayfield::BreakBreakable(const FVector& center)
{
	int32 id=CheckBreakable(center);
	if(id<0)
		return false;
	Breakable->RemoveInstance(id);
	SpawnPickup(center);
	return true;
}
// Called when the game starts or when spawned
void APlayfield::BeginPlay()
{
	Super::BeginPlay();
	initializeGame=true;
	CreateMap();
	
}
void	APlayfield::KillPlayers()
{
	for(int i=0;i<spawned.Num();i++)
	{
		GWorld->DestroyActor(spawned[i]);
	}
	spawned.SetNum(0,false);
	memset(players,0,sizeof(players));
}

ABMPlayer*	APlayfield::SpawnPlayer(UClass* tempclass, const FTransform &transform)
{
	FActorSpawnParameters spwn;
	spwn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABMPlayer*p =GWorld->SpawnActor<ABMPlayer>(tempclass,transform,spwn);
	if(p)
	{
		p->SetMain(this);
		spawned.Add(p);
		p->AttachToComponent(Root,FAttachmentTransformRules::KeepRelativeTransform);
	}
	return p;
}

void	APlayfield::SpawnPlayers()
{
	FTransform tr;
	FVector halfSize=FVector(YSize,XSize,0)*50.f;
	if(TemplatePlayer1)
	{
		FVector topleft(halfSize.X,-halfSize.Y,0);
		tr.SetLocation(topleft+GetActorLocation());
		players[0]=SpawnPlayer(TemplatePlayer1,tr);
	}
	if(TemplatePlayer2)
	{
		FVector topright(halfSize.X,halfSize.Y,0);
		tr.SetLocation(topright+GetActorLocation()+FVector(0,-100,0));
		players[1]=SpawnPlayer(TemplatePlayer2,tr);
	}
	if(TemplatePlayer3)
	{
		FVector botleft(-halfSize.X,-halfSize.Y,0);
		tr.SetLocation(botleft+GetActorLocation()+FVector(100,0,0));
		players[2]=SpawnPlayer(TemplatePlayer3,tr);
	}
	if(TemplatePlayer4)
	{
		FVector botright(-halfSize.X,halfSize.Y,0);
		tr.SetLocation(botright+GetActorLocation()+FVector(100,-100,0));
		players[3]=SpawnPlayer(TemplatePlayer4,tr);
	}
}


// Called every frame
void APlayfield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(initializeGame)
	{
		KillPlayers();
		SpawnPlayers();
		initializeGame=false;

	}
}

// Called to bind functionality to input
void APlayfield::SetupPlayerInputComponent(UInputComponent* ic)
{
	Super::SetupPlayerInputComponent(ic);
	ic->BindAction("Human1Fire",IE_Released,this,&APlayfield::Fire1);
	ic->BindAxis("Human1Up",this,&APlayfield::Up1);
	ic->BindAxis("Human1Right",this,&APlayfield::Right1);

	ic->BindAction("Human2Fire",IE_Released,this,&APlayfield::Fire2);
	ic->BindAxis("Human2Up",this,&APlayfield::Up2);
	ic->BindAxis("Human2Right",this,&APlayfield::Right2);
}

void APlayfield::Fire1()
{
	if(!players[0])
		return;
	players[0]->Fire();

}
void APlayfield::Up1(float amount)
{
	if(!players[0])
		return;
	players[0]->Up(amount);

}
void APlayfield::Right1(float amount)
{
	if(!players[0])
		return;
	players[0]->Right(amount);

}
void APlayfield::Fire2()
{
	if(!players[1])
		return;
	players[1]->Fire();


}
void APlayfield::Up2(float amount)
{
	if(!players[1])
		return;
	players[1]->Up(amount);

}
void APlayfield::Right2(float amount)
{
	if(!players[1])
		return;
	players[1]->Right(amount);

}


void APlayfield::SpawnPickup(const FVector& pos)
{
	if(FMath::RandRange(0,100)>PickupSpawnPercetage)
		return;
	int32 numPickups=Pickups.Num();
	if(!numPickups)
		return;
	int32 idx=(FMath::Rand()*numPickups)/RAND_MAX;
	FTransform tr;
	tr.SetLocation(pos);
	FActorSpawnParameters spwn;
	spwn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABMPickup*p =GWorld->SpawnActor<ABMPickup>(Pickups[idx],tr,spwn);
	p->AttachToComponent(Root,FAttachmentTransformRules::KeepRelativeTransform);
}
void APlayfield::SpawnTemplate(class UClass* tempClass, const FVector& pos, AActor* SpawnOwner)
{
	FTransform tr;
	tr.SetLocation(pos);
	FActorSpawnParameters spwn;
	spwn.Owner=SpawnOwner;
//	spwn.bNoFail=true;
	spwn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor*p =GWorld->SpawnActor(tempClass,&pos,0,spwn);
	if(p)
		p->AttachToComponent(Root,FAttachmentTransformRules::KeepRelativeTransform);
	int i=0;
}
void	APlayfield::RemovePlayer(ABMPlayer* player)
{
	if(players[0]==player)
		players[0]=nullptr;
	else if(players[1]==player)
		players[1]=nullptr;
	else if(players[2]==player)
		players[2]=nullptr;
	else if(players[3]==player)
		players[3]=nullptr;
}
bool	APlayfield::IsPlayerAlive(int32 player)
{
	if(!spawned.Num())
		return false;
	return players[player]!=0;
}
int32	APlayfield::GetPlayerBombs(int32 player)
{
	if(!spawned.Num())
		return 0;
	return spawned[player]->Bombs;
}
int32	APlayfield::GetPlayerAvailableBombs(int32 player)
{
	if(!spawned.Num())
		return 0;
	return spawned[player]->GetAvailableBombs();
}


#pragma optimize("",on)

