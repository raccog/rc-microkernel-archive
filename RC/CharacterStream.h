#pragma once

#include "CharacterReader.h"
#include "CharacterWriter.h"

class CharacterStream : public CharacterWriter, public CharacterReader {};
