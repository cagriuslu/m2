#pragma once
#include <Simulation.pb.h>
#include <m2/Object.h>

int64_t AnimalAllocator(pinball::pb::Animal_Type type);
void AnimalDeallocator(int64_t id);
