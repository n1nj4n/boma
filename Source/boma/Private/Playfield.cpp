// Fill out your copyright notice in the Description page of Project Settings.

#include "Playfield.h"
#include "bmplayer.h"
#include "bmpickup.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

//#pragma optimize("",on)

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

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TemplatePlayer1=nullptr;
	TemplatePlayer2=nullptr;
	TemplatePlayer3=nullptr;
	TemplatePlayer4=nullptr;
	XSize=15;
	YSize=15;
	memset(players,0,sizeof(players));

	NoLevelBuilding=false;

	PickupSpawnPercetage=30.f;
	GameTime=120.f;
	gameOver=true;
	gameTimer=0;
	CameraSpeed=400;
	placedBombs=1;
	SoftWallDistribution=50.f;

}
// Called when the game starts or when spawned
void APlayfield::BeginPlay()
{
	Super::BeginPlay();
	CreateMap();
}

// Called to bind functionality to input
void APlayfield::SetupPlayerInputComponent(UInputComponent* ic)
{
	Super::SetupPlayerInputComponent(ic);
	ic->BindAction("Human1Fire",IE_Pressed,this,&APlayfield::Fire1);
	ic->BindAxis("Human1Up",this,&APlayfield::Up1);
	ic->BindAxis("Human1Right",this,&APlayfield::Right1);

	ic->BindAction("Human2Fire",IE_Pressed,this,&APlayfield::Fire2);
	ic->BindAxis("Human2Up",this,&APlayfield::Up2);
	ic->BindAxis("Human2Right",this,&APlayfield::Right2);
}

// creates the game area
void APlayfield::CreateMap()
{
	if(NoLevelBuilding)
	{
		xS=4;
		yS=4;
		return;
	}
	Unbreakable->ClearInstances();
	Breakable->ClearInstances();

	// add some random to the size of the playfield
	xS=XSize+(FMath::Rand()>>13)-4;
	yS=YSize+(FMath::Rand()>>13)-4;
	// make sure we create the playfield in right size
	// needs to be uneven to create the playfield properly
	xS&=~1;
	yS&=~1;
	xS++;
	yS++;

	// allocate and initialize our workarea
	// used later when we create our breakable walls
	uint8* tileAlloc=new uint8[xS*yS];
	memset(tileAlloc,0,xS*yS);

	// ** allocate space for players to spawn
	// *  player 1
	tileAlloc[0]=1;
	tileAlloc[1]=1;
	tileAlloc[xS]=1;

	// **
	//  * player 2
	tileAlloc[xS-1]=1;
	tileAlloc[xS-2]=1;
	tileAlloc[xS*2-1]=1;

	// *
	// ** player 3
	int offset=xS*(yS-2);	// second last row.
	tileAlloc[offset]=1;
	tileAlloc[offset+xS]=1;
	tileAlloc[offset+xS+1]=1;

	//  *
	// ** player 4
	offset+=xS;			    // add one row, compensate below with subtracting 1 to offset 
	tileAlloc[offset-1]=1;
	tileAlloc[offset+xS-2]=1;
	tileAlloc[offset+xS-1]=1;

	// randomize our allocation table with fake usage, creating holes in the map
	for(int i=0;i<xS*yS;i++)
	{
		if(FMath::RandRange(0,1)>(SoftWallDistribution/100.f))
			tileAlloc[i]=1;

	}
	// create surrounding wall
	FVector realSize=FVector(yS,xS,0)*tileSize;
	FVector halfSize=realSize*0.5f;
	FTransform tr;
	FVector topleft(halfSize.X+tileSize,-halfSize.Y,0);
	FVector botleft(-halfSize.X,-halfSize.Y,0);
	// left <-> right walls around the playfirld
	for(int x=0;x<xS;x++)
	{
		// create top unbreakable wall
		tr.SetLocation(topleft);
		Unbreakable->AddInstance(tr);
		topleft.Y+=tileSize;
		// create bottom unbreakable wall
		tr.SetLocation(botleft);
		Unbreakable->AddInstance(tr);
		botleft.Y+=tileSize;
	}
	// up <-> down walls around playfield
	topleft=FVector(halfSize.X,-halfSize.Y-tileSize,0);
	FVector topright(halfSize.X,halfSize.Y,0);
	for(int y=0;y<yS;y++)
	{
		// create left unbreakable wall
		tr.SetLocation(topleft);
		Unbreakable->AddInstance(tr);
		topleft.X-=tileSize;

		// create right unbreakable wall
		tr.SetLocation(topright);
		Unbreakable->AddInstance(tr);
		topright.X-=tileSize;
	}
	// place unbreakable walls on the main play field
	topleft=FVector(halfSize.X,-halfSize.Y,0);
	for(int y=1;y<yS-1;y++)
	{
		for(int x=1;x<xS-1;x++)
		{
			// place a unbreakable wall when both x and y are uneven.
			if(x&1 && y&1)
			{
				tileAlloc[xS*y+x]=1;
				FVector pos(-y,x,0);
				pos*=tileSize;
				tr.SetLocation(topleft+pos);
				Unbreakable->AddInstance(tr);
			}
		}
	}
	// place destructible walls where there is nothing else allocated
	for(int y=0;y<yS;y++)
	{
		for(int x=0;x<xS;x++)
		{
			if(!tileAlloc[xS*y+x])
			{
				FVector pos(-y,x,-0.15);
				pos*=tileSize;
				tr.SetLocation(topleft+pos);
				Breakable->AddInstance(tr);
			}
		}
	}
	delete[] tileAlloc;
}


void	APlayfield::RemovePlayers()
{
	for(int i=0;i<spawned.Num();i++)
	{
		GWorld->DestroyActor(spawned[i]);
	}
	spawned.SetNum(0,false);
	memset(players,0,sizeof(players));
}
// Spawn a player
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
// Spawn players for next round
void	APlayfield::SpawnPlayers()
{
	FTransform tr;
	FVector halfSize=FVector(yS,xS,0)*tileSize*0.5f;
	if(TemplatePlayer1)
	{
		FVector topleft(halfSize.X,-halfSize.Y,0);
		tr.SetLocation(topleft+GetActorLocation());
		players[0].player=SpawnPlayer(TemplatePlayer1,tr);
	}
	if(TemplatePlayer2)
	{
		FVector topright(halfSize.X,halfSize.Y,0);
		tr.SetLocation(topright+GetActorLocation()+FVector(0,-tileSize,0));
		players[1].player=SpawnPlayer(TemplatePlayer2,tr);
	}
	if(TemplatePlayer3)
	{
		FVector botleft(-halfSize.X,-halfSize.Y,0);
		tr.SetLocation(botleft+GetActorLocation()+FVector(tileSize,0,0));
		players[2].player=SpawnPlayer(TemplatePlayer3,tr);
	}
	if(TemplatePlayer4)
	{
		FVector botright(-halfSize.X,halfSize.Y,0);
		tr.SetLocation(botright+GetActorLocation()+FVector(tileSize,-tileSize,0));
		players[3].player=SpawnPlayer(TemplatePlayer4,tr);
	}
}
// find out who's the winner
void APlayfield::CalcWinner()
{

	int32 aliveCount=0;
	// count alive players
	for(int i=0;i<numPlayers;i++)
	{
		if(players[i].player)
			aliveCount++;
	}
	if(gameTimer==0)
	{
		if(aliveCount>1)
		{
			lastWinner=FString("Time out");
			return;
		}
	}
	if(aliveCount==1)
	{
		for(int i=0;i<numPlayers;i++)
		{
			if(players[i].player)
			{
				lastWinner=FString::Printf(TEXT("Player %d"),i);
				return;
			}
		}
	}

	// if all dead, check if their life span
	int32 player=0;
	bool same=false;
	for(int i=1;i<numPlayers;i++)
	{
		if(players[i].lifeTime==players[player].lifeTime)
			same=true;
		if(players[i].lifeTime>players[player].lifeTime)
			player=i;
	}
	if(same)
		lastWinner=FString("Draw");
	else
		lastWinner=FString::Printf(TEXT("Player %d"),player);
}

// update camera to fit players on the screen
void APlayfield::CameraUpdate(float DeltaTime)
{
	if(APlayerController* pC=Cast<APlayerController>(GetController()))
	{
		int32 x,y;
		pC->GetViewportSize(x,y);
		FVector vp(x,x,1);
		if(y<x)vp=FVector(y,y,1);
		FVector rvp(x,y,0);
		FVector hvp=rvp*0.5f;
		FVector mi(10000, 10000,0);
		FVector ma(-10000,-10000,1);

		FVector mi3(10000, 10000, 0);
		FVector ma3(-10000,-10000,1);

		float scale=rvp.X/rvp.Y;
		if(rvp.X>rvp.Y)scale=rvp.Y/rvp.X;

		// get approximated bound around players
		FVector pr;
		pr.Z=1;
		FVector half(tileSize,tileSize,0);
		half*=0.5f;
		for(int i=0;i<spawned.Num();i++)
		{
			FVector pos=spawned[i]->GetActorLocation();

			pC->ProjectWorldLocationToScreenWithDistance(pos+half,pr);

			if(mi.X>pr.Y)mi.X=pr.Y;
			if(ma.X<pr.Y)ma.X=pr.Y;
			if(mi.Y>pr.X)mi.Y=pr.X;
			if(ma.Y<pr.X)ma.Y=pr.X;

			pC->ProjectWorldLocationToScreenWithDistance(pos-half,pr);
			if(mi.X>pr.Y)mi.X=pr.Y;
			if(ma.X<pr.Y)ma.X=pr.Y;
			if(mi.Y>pr.X)mi.Y=pr.X;
			if(ma.Y<pr.X)ma.Y=pr.X;



			if(mi3.X>pos.X)mi3.X=pos.X;
			if(ma3.X<pos.X)ma3.X=pos.X;
			if(mi3.Y>pos.Y)mi3.Y=pos.Y;
			if(ma3.Y<pos.Y)ma3.Y=pos.Y;

		}
		// update camera location
		FVector size=ma-mi;
		FVector sizeOriginal=ma3-mi3;
		FVector center=(ma3+mi3)*0.5f;
		size=FVector(1.f)/(size/vp);

		if(rvp.X<rvp.Y)size.Y*=scale;
		else size.X*=scale;
		FVector p=Camera->GetComponentLocation();

		if(size.X>0.9f && size.Y>0.9f)
			p.Z-=CameraSpeed*DeltaTime;
		else if(size.X<0.7f || size.Y<0.7f)
			p.Z+=CameraSpeed*DeltaTime;
		p.X=center.X;
		p.Y=center.Y;
		Camera->SetRelativeLocation(p);
	}
}

// Called every frame
void APlayfield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(!gameOver)
	{
		// update timers
		CameraUpdate(DeltaTime);
		gameTimer-=DeltaTime;
		for(int i=0;i<numPlayers;i++)
		{
			players[i].lifeTime+=DeltaTime;
		}
		if(gameTimer<0)
		{
			// time is up.. do end game
			gameTimer=0;
			gameOver=true;
			for(int i=0;i<numPlayers;i++)
			{
				if(players[i].player)
					players[i].player->disable();
			}
		}
		else
		{
			int n=0;
			for(int i=0;i<numPlayers;i++)
			{
				if(players[i].player)
					n++;
			}
			if(n<2)
			{
				// less than 2 left.. trigger end round
				gameOver=true;
			}
		}
	}
	else
	{
		// make sure all bombs have detonated
		if(!placedBombs)
		{
			CalcWinner();

			gameTimer=0;
			for(int i=0;i<numPlayers;i++)
			{
				if(players[i].player)
					players[i].player->disable();
			}
			memset(players,0,sizeof(players));
			// nasty..
			placedBombs++;
		}
	}
}


void APlayfield::InitializeGame()
{
	CreateMap();
	RemovePlayers();
	memset(players,0,sizeof(players));
	SpawnPlayers();
	placedBombs=0;
	gameTimer=GameTime;
	gameOver=false;
}

// Player 1 inputs
void APlayfield::Fire1()
{
	if(gameOver)
	{
		InitializeGame();
		return;
	}
	if(!players[0].player)
		return;
	players[0].player->Fire();

}
void APlayfield::Up1(float amount)
{
	if(!players[0].player)
		return;
	players[0].player->Up(amount);

}
void APlayfield::Right1(float amount)
{
	if(!players[0].player)
		return;
	players[0].player->Right(amount);

}

// Player 2 inputs
void APlayfield::Fire2()
{
	if(gameOver)
	{
		InitializeGame();
		return;
	}
	if(!players[1].player)
		return;
	players[1].player->Fire();


}
void APlayfield::Up2(float amount)
{
	if(!players[1].player)
		return;
	players[1].player->Up(amount);

}
void APlayfield::Right2(float amount)
{
	if(!players[1].player)
		return;
	players[1].player->Right(amount);

}

void APlayfield::SpawnPickup(const FVector& pos)
{
	if(gameOver)
		return;
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
	if(p)
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
}
void	APlayfield::DisablePlayer(ABMPlayer* player)
{
	if(players[0].player==player)
		players[0].player=nullptr;
	else if(players[1].player==player)
		players[1].player=nullptr;
	else if(players[2].player==player)
		players[2].player=nullptr;
	else if(players[3].player==player)
		players[3].player=nullptr;
}

// UI helpers

bool	APlayfield::IsPlayerAlive(int32 player)
{
	if(!spawned.Num())
		return false;
	return players[player].player!=0;
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

FString	APlayfield::GetGameTime()
{
	int minute=gameTimer/60;
	int sec=gameTimer-minute*60;
	return FString::Printf(TEXT("%02d:%02d"),minute,sec);
}

bool	APlayfield::IsGameOver()
{
	return gameOver;

}
FString	APlayfield::GetLastWinner()
{
	return lastWinner;
}

////
// AI helpers
////

// check if any unbreakable walls at location
bool APlayfield::CheckUnbreakable(const FVector& location)
{
	TArray<int32> idx;
	idx=Unbreakable->GetInstancesOverlappingSphere(location, 10.5f);
	return idx.Num()!=0;
}
// returns id of breakable wall at location
int32 APlayfield::CheckBreakable(const FVector& location)
{
	TArray<int32> idx;
	idx=Breakable->GetInstancesOverlappingSphere(location, 10.5f);
	return idx.Num()?idx[0]:-1;
}
// checks for any walls at location
bool	APlayfield::AnyWall(const FVector& location)
{
	if(CheckBreakable(location)>=0)
		return  true;
	return CheckUnbreakable(location);
}
// break wall at location
bool APlayfield::BreakBreakable(const FVector& location)
{
	int32 id=CheckBreakable(location);
	if(id<0)
		return false;
	Breakable->RemoveInstance(id);
	SpawnPickup(location);
	return true;
}
// check any breakable walls around an actor.
bool APlayfield::AnyBreakableAround(AActor* obj)
{
	FVector loc=obj->GetActorLocation();
	int up	 =CheckBreakable(loc + FVector(100,0,0));
	if(up>=0)
		return true;
	int down =CheckBreakable(loc + FVector(-100,0,0));
	if(down>=0)
		return true;
	int left =CheckBreakable(loc + FVector(0,-100,0));
	if(left>=0)
		return true;
	int right=CheckBreakable(loc + FVector(0,100,0));
	if(right>=0)
		return true;
	return false;
}
// get closest actor of a type within radius 
AActor* APlayfield::ClosestOfClass(AActor* obj, UClass* type, float radius)
{
	ULevel* pLev=GWorld->GetLevel(0);
	AActor* closest=nullptr;
	float sd=radius*radius;
	FVector loc=obj->GetActorLocation();
	for(int i=0;i<pLev->Actors.Num();i++)
	{
		AActor* a=pLev->Actors[i];
		if(!a)
			continue;
		UClass* ac=a->GetClass();
		if(ac==type)
		{
			FVector distance=a->GetActorLocation()-loc;
			float distSq=distance.SizeSquared();
			if(sd>distSq)
			{
				sd=distSq;
				closest=a;
			}
		}
	}
	return closest;
}
// get array of sorted actors of specific classes within radius.
TArray<AActor*> APlayfield::GetSortedOfClasses(AActor* obj, const TArray<UClass*> &types, float radius)
{
	TArray<AActor*> retActors;
	if(!obj)
		return retActors;
	TArray<float> retActorsDistanceSquare;
	ULevel* pLev=GWorld->GetLevel(0);
	AActor* closest=nullptr;
	float shortestDistance=radius*radius;
	FVector loc=obj->GetActorLocation();
	for(int i=0;i<pLev->Actors.Num();i++)
	{
		AActor* a=pLev->Actors[i];
		if(!a)
			continue;
		for(int j=0;j<types.Num();j++)
		{
			if(a->GetClass()==types[j])
			{
				FVector distance=a->GetActorLocation()-loc;
				float adSquareDistance=distance.SizeSquared();
				if(adSquareDistance>shortestDistance)
					continue;
				if(!retActors.Num())
				{
					retActors.Add(a);
					retActorsDistanceSquare.Add(adSquareDistance);
					continue;
				}
				for(int k=0;k<retActorsDistanceSquare.Num();k++)
				{
					if(retActorsDistanceSquare[k]>adSquareDistance)
					{
						retActorsDistanceSquare.Insert(adSquareDistance,k);
						retActors.Insert(a,k);
						break;
					}
				}
				break;
			}
		}
	}
	return retActors;
}
// get distance of closest actor of type
float APlayfield::DistanceOfClosestOfClass(AActor* obj, UClass* type, float radius)
{
	ULevel* pLev=GWorld->GetLevel(0);
	AActor* closest=nullptr;
	float sd=radius*radius;
	FVector loc=obj->GetActorLocation();
	for(int i=0;i<pLev->Actors.Num();i++)
	{
		AActor* a=pLev->Actors[i];
		if(a->GetClass()==type)
		{
			FVector distance=a->GetActorLocation()-loc;
			float distSq=distance.SizeSquared();
			if(sd>distSq)
			{
				sd=distSq;
				closest=a;
			}
		}
	}
	return FMath::Sqrt(sd);
}
// returns valid step directions
TArray<FVector> APlayfield::ValidStepDirections(AActor* actor)
{
	TArray<FVector> ret;
	if(!actor)
		return ret;
	FRotator r(0,actor->GetActorRotation().Yaw,0);
	FVector frontdir=r.RotateVector(FVector(100,0,0));
	FVector rightdir=r.RotateVector(FVector(0,100,0));
	FVector pos=actor->GetActorLocation();
	FVector directions[4]={frontdir,-rightdir,rightdir,-frontdir};
	for(int i=0;i<4;i++)
	{
		if(!AnyWall(pos+directions[i]))
		{
			ret.Add(directions[i]);
		}
	}
	return ret;
}

//#pragma optimize("",on)

