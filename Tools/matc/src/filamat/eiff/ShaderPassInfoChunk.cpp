#include "ShaderPassInfoChunk.h"
#include "filaflat/MaterialChunkType.h"
#include "filaflat/ShaderPassInfo.h"
#include <utility>
#include <stdint.h>

using namespace filament;

namespace filamat
{
ShaderPassInfoChunk::ShaderPassInfoChunk(filament::ShaderPassInfo & uib)
    : Chunk(MaterialPassInfo)
    , mPass(uib)
{
}
void ShaderPassInfoChunk::flatten(Flattener&f)
{
    auto& pass=mPass.getPasses();
    auto& feature = mPass.getFeatures();
    
    f.writeUint64(pass.size());
    for( int i = 0; i < pass.size(); i++ )
    {
        f.writeString(pass[i]);
        
        f.writeUint64(feature[i].size());
        for( auto& it : feature[i] )
        {
            f.writeString(it.first);
            f.writeUint64(it.second.size());
            for( int j = 0; j < it.second.size(); j++ )
            {
                f.writeString(it.second[j]);
            }
        }

    }
}
} // namespace filamat
