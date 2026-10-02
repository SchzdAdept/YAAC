#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	// Explicit timer guarantee: Self-destruct after 2.5 seconds regardless of lifespan bugs
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ABullet::SelfDestruct, 0.5f, false);
}

void ABullet::SelfDestruct()
{
	Destroy();
}


ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Collision setup
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComp;

	// 2. Visual Mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	// 3. Projectile Movement Component (handles straight-line flight)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // No gravity in space

	// Auto-destroy after 2 seconds so bullets don't clog memory
	InitialLifeSpan = 2.0f; 
}

void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector bounds = FVector(600.0f, 1050.0f, 0.0f);
	FVector location = GetActorLocation();
	bool bWrapped = false;

	if (location.X > bounds.X)  { location.X -= 2 * bounds.X; bWrapped = true; }
	if (location.X < -bounds.X) { location.X += 2 * bounds.X; bWrapped = true; }
	if (location.Y > bounds.Y)  { location.Y -= 2 * bounds.Y; bWrapped = true; }
	if (location.Y < -bounds.Y) { location.Y += 2 * bounds.Y; bWrapped = true; }

	if (bWrapped)
	{
		// 1. Move actor to wrapped location without triggering sweep collisions
		SetActorLocation(location, false, nullptr, ETeleportType::TeleportPhysics);

		// 2. Re-apply velocity to Projectile Movement Component so it doesn't freeze
		if (ProjectileMovement)
		{
			ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
			ProjectileMovement->UpdateComponentVelocity();
		}
	}
}