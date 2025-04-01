#include "FGBuildableReactiveOreExtractor.h"
#include "FGInventoryComponent.h"
#include "Resources/FGResourceNode.h"
#include "FGFactoryConnectionComponent.h"

AFGBuildableReactiveOreExtractor::AFGBuildableReactiveOreExtractor() : Super()
{
    mInputInventory = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("InputInventory"));
}

void AFGBuildableReactiveOreExtractor::BeginPlay()
{
    Super::BeginPlay();
    // Set up input inventory
    mItemsPerCycle = 6;
    mInputInventory->SetReplicationRelevancyOwner(this);
    GetInputInventory()->SetAllowedItemOnIndex(0, mCatalystItemClass);
    GetOutputInventory()->SetAllowedItemOnIndex(0, mOutputItemClass);
    GetOutputInventory()->RemoveArbitrarySlotSize(0);
    ForEachComponent<UFGFactoryConnectionComponent>(true, [&](UFGFactoryConnectionComponent* FactoryConn) {
        switch (FactoryConn->GetDirection()) {
        case EFactoryConnectionDirection::FCD_INPUT:
            FactoryConn->SetInventory(GetInputInventory());
            FactoryConn->SetInventoryAccessIndex(0);
            break;
        case EFactoryConnectionDirection::FCD_OUTPUT:
            FactoryConn->SetInventory(GetOutputInventory());
            FactoryConn->SetInventoryAccessIndex(0);
            break;
        }
        });
    // It would be better to cache connections but my pc is weak - can't test, don't care
}
bool AFGBuildableReactiveOreExtractor::CanProduce_Implementation() const
{
    // All the imaginable checks
    FInventoryStack Stack = FInventoryStack(mItemsPerCycle, mOutputItemClass);
    return HasPower() && !IsProductionPaused() && mInputInventory->HasItems(mCatalystItemClass, mCatalystItemsPerCycle)
        && GetOutputInventory()->HasEnoughSpaceForStack(Stack);
}

static FInventoryStack GrabOutputFromFactory(UFGFactoryConnectionComponent* Comp, TSubclassOf<UFGItemDescriptor> type, int32 space)
{
    float Offset;
    FInventoryItem Item;
    if (not Comp->Factory_GrabOutput(Item, Offset, type)) {
        return FInventoryStack();
    }
    if (Item.HasState()) {
        return FInventoryStack(Item);
    }
    int32 i;
    for (i = 1; i < Item.GetItemStackSize(); i++) {
        if (not Comp->Factory_GrabOutput(Item, Offset, type) || space <= 0) {
            break;
        }
        space--;
    }
    return FInventoryStack(i, type);
}

void AFGBuildableReactiveOreExtractor::Factory_Tick(float dt)
{
    Super::Factory_Tick(dt);
    if (not HasAuthority() || IsProductionPaused()) {
        return;
    }

    //Grab Items into input inventory
    UFGFactoryConnectionComponent* InputConnection;
    ForEachComponent<UFGFactoryConnectionComponent>(false, [&](UFGFactoryConnectionComponent* FactoryConn) {
        if (FactoryConn->GetDirection() == EFactoryConnectionDirection::FCD_INPUT) {
            InputConnection = FactoryConn;
        }
        });
        int32 space = GetInputInventory()->GetSlotSize(0, mCatalystItemClass) - GetInputInventory()->GetNumItems(mCatalystItemClass);
        FInventoryStack Stack;
        if (space > 0) Stack = GrabOutputFromFactory(InputConnection, mCatalystItemClass, space);
        if (Stack.HasItems()) {
        mInputInventory->AddStackToIndex(0, Stack);
    }
}

void AFGBuildableReactiveOreExtractor::Factory_TickProducing(float dt)
{
    //Not checking CanProduce(), it seems to be used outside of this method actually, maybe at Factory_Tick
    Super::Super::Factory_TickProducing(dt);
    if (not HasAuthority()) {
        return;
    }
    
    //Actual production
    mCurrentExtractProgress += (1.0f / CalcProductionCycleTimeForPotential(GetCurrentPotential())) * dt;
    if (mCurrentExtractProgress >= 1.0f)
    {
        mInputInventory->RemoveFromIndex(0, mCatalystItemsPerCycle);
        FInventoryStack Stack = FInventoryStack(mItemsPerCycle, mOutputItemClass);
        GetOutputInventory()->AddStackToIndex(0, Stack);
        Factory_ProductionCycleCompleted(mCurrentExtractProgress - 1.0f);
        mCurrentExtractProgress -= 1.0f;
    }
}

float AFGBuildableReactiveOreExtractor::GetCatalystConsumptionPerMinute() const
{
    const float cycleTime = GetProductionCycleTime();
    if (cycleTime > SMALL_NUMBER)
    {
        return (mCatalystItemsPerCycle / cycleTime) * 60;
    }
    return 0.0f;
}

float AFGBuildableReactiveOreExtractor::GetNumExtractedItemsPerMinute() const
{
    const float cycleTime = GetProductionCycleTime();
    if (cycleTime > SMALL_NUMBER)
    {
        return (mItemsPerCycle / cycleTime) * 60;
    }
    return 0.0f;
}