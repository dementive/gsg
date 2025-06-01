#pragma once

#include <cstdint>
#include "singleton.hpp"

using Entity = uint32_t;
constexpr uint32_t ENTITY_MAX = UINT32_MAX;

using SmallEntity = uint16_t;
constexpr uint16_t SMALL_ENTITY_MAX = UINT16_MAX;

using TinyEntity = uint8_t;
constexpr uint8_t TINY_ENTITY_MAX = UINT8_MAX;

// All entity types

using ProvinceEntity = SmallEntity;
using ProvinceBorderEntity = Entity;
using ProvinceAdjacencyEntity = Entity;
using ProvinceCrossingEntity = SmallEntity;

using AreaEntity = SmallEntity;
using RegionEntity = SmallEntity;
using CountryEntity = Entity;
