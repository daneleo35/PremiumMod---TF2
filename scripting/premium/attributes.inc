// Runtime attribute ownership, shared with fault-injection VM tests.
enum struct AttributeState
{
    bool owned;
    bool existed;
    int reference;
    float original;
    float applied;
}

void RestoreAttribute(AttributeState state, const char[] name)
{
    if (!state.owned) return;
    // Also safe during failed startup or unexpected provider unloading.
    if (!PremiumAttributeNativesAvailable()) return;
    int entity = EntRefToEntIndex(state.reference);
    if (entity != INVALID_ENT_REFERENCE && IsValidEntity(entity))
    {
        Address address = TF2Attrib_GetByName(entity, name);
        if (address != Address_Null && FloatAbs(TF2Attrib_GetValue(address) - state.applied) < 0.0001)
        {
            if (state.existed) TF2Attrib_SetByName(entity, name, state.original);
            else TF2Attrib_RemoveByName(entity, name);
            if (entity > MaxClients && HasEntProp(entity, Prop_Send, "m_hOwnerEntity"))
            {
                int owner = GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
                if (ValidClient(owner)) TF2Attrib_ClearCache(owner);
            }
        }
    }
    state.owned = false;
}

void ApplyAttribute(int entity, AttributeState state, const char[] name, float multiplier)
{
    if (multiplier == 1.0)
    {
        RestoreAttribute(state, name);
        return;
    }
    int reference = EntIndexToEntRef(entity);
    if (state.owned && reference != state.reference) RestoreAttribute(state, name);
    if (!state.owned)
    {
        Address address = TF2Attrib_GetByName(entity, name);
        state.existed = address != Address_Null;
        state.original = state.existed ? TF2Attrib_GetValue(address) : 1.0;
        state.reference = reference;
        state.owned = true;
    }
    float desired = state.original * multiplier;
    Address current = TF2Attrib_GetByName(entity, name);
    if (current != Address_Null && FloatAbs(TF2Attrib_GetValue(current) - desired) < 0.0001)
    {
        state.applied = desired;
        return;
    }
    state.applied = desired;
    if (!TF2Attrib_SetByName(entity, name, state.applied)) state.owned = false;
    else if (entity > MaxClients && HasEntProp(entity, Prop_Send, "m_hOwnerEntity"))
    {
        int owner = GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
        if (ValidClient(owner)) TF2Attrib_ClearCache(owner);
    }
}
