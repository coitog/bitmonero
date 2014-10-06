// Copyright (c) 2014, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include <algorithm>

#include "include_base_utils.h"
using namespace epee;

#include "checkpoints.h"

namespace cryptonote
{
  //---------------------------------------------------------------------------
  checkpoints::checkpoints()
  {
  }
  //---------------------------------------------------------------------------
  bool checkpoints::add_checkpoint(uint64_t height, const std::string& hash_str, bool is_long_hash)
  {
    crypto::hash h = null_hash;
    bool r = epee::string_tools::parse_tpod_from_hex_string(hash_str, h);
    CHECK_AND_ASSERT_MES(r, false, "Failed to parse checkpoint hash string into binary representation!");

    if (!is_long_hash)
    {
      // return false if adding at a height we already have AND the hash is different
      if (m_points.count(height))
      {
        CHECK_AND_ASSERT_MES(h == m_points[height], false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
      }
      m_points[height] = h;
      return true;
    }
    else
    {
      // return false if adding at a height we already have AND the hash is different
      if (m_points_long.count(height))
      {
        CHECK_AND_ASSERT_MES(h == m_points_long[height], false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
      }
      m_points_long[height] = h;
      return true;
    }
    return true;  // can't ever get here, but compiler might complain
  }
  //---------------------------------------------------------------------------
  bool checkpoints::is_in_checkpoint_zone(uint64_t height) const
  {
    return !m_points.empty() && ((height <= (--m_points.end())->first) || (height <= (--m_points_long.end())->first));
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_block(uint64_t height, const crypto::hash& h, bool& is_a_checkpoint, bool is_long_hash) const
  {
    std::map<uint64_t, crypto::hash>::const_iterator it;
    if (is_long_hash)
    {
      it = m_points_long.find(height);
      is_a_checkpoint = it != m_points_long.end();
    }
    else
    {
      it = m_points.find(height);
      is_a_checkpoint = it != m_points.end();
    }

    std::string hash_type = is_long_hash ? "LONGHASH " : "";

    if(!is_a_checkpoint)
      return true;

    if(it->second == h)
    {
      LOG_PRINT_GREEN(hash_type << "CHECKPOINT PASSED FOR HEIGHT " << height << " " << h, LOG_LEVEL_1);
      return true;
    }else
    {
      LOG_ERROR(hash_type << "CHECKPOINT FAILED FOR HEIGHT " << height << ". EXPECTED HASH: " << it->second << ", FETCHED HASH: " << h);
      return false;
    }
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_block(uint64_t height, const crypto::hash& h, bool is_long_hash) const
  {
    bool ignored;
    return check_block(height, h, ignored, is_long_hash);
  }
  //---------------------------------------------------------------------------
  bool checkpoints::is_alternative_block_allowed(uint64_t blockchain_height, uint64_t block_height) const
  {
    if (0 == block_height)
      return false;

    auto it = m_points.upper_bound(blockchain_height);
    auto it_long = m_points_long.upper_bound(blockchain_height);
    // Is blockchain_height before the first checkpoint?
    if (it_long == m_points_long.begin() && it == m_points.begin())
    {
      return true;
    }
    else if (it_long == m_points_long.begin())
    {
      --it;
      return it->first < block_height;
    }
    else if (it == m_points.begin())
    {
      --it_long;
      return it_long->first < block_height;
    }

    --it;
    --it_long;

    uint64_t checkpoint_height = std::max(it->first, it_long->first);
    return checkpoint_height < block_height;
  }
  //---------------------------------------------------------------------------
  uint64_t checkpoints::get_max_height()
  {
    std::map< uint64_t, crypto::hash >::const_iterator highest = 
        std::max_element( m_points.begin(), m_points.end(),
                         ( boost::bind(&std::map< uint64_t, crypto::hash >::value_type::first, _1) < 
                           boost::bind(&std::map< uint64_t, crypto::hash >::value_type::first, _2 ) ) );
    std::map< uint64_t, crypto::hash >::const_iterator highest_long = 
        std::max_element( m_points_long.begin(), m_points_long.end(),
                         ( boost::bind(&std::map< uint64_t, crypto::hash >::value_type::first, _1) < 
                           boost::bind(&std::map< uint64_t, crypto::hash >::value_type::first, _2 ) ) );
    return (highest->first > highest_long->first) ? highest->first : highest_long->first;
  }
  //---------------------------------------------------------------------------
  const std::map<uint64_t, crypto::hash>& checkpoints::get_points()
  {
    return m_points;
  }
  //---------------------------------------------------------------------------
  const std::map<uint64_t, crypto::hash>& checkpoints::get_points_long()
  {
    return m_points_long;
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_for_conflicts(checkpoints& other)
  {
    for (auto& pt : other.get_points())
    {
      if (m_points.count(pt.first))
      {
        CHECK_AND_ASSERT_MES(pt.second == m_points[pt.first], false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
      }
    }
    for (auto& pt : other.get_points_long())
    {
      if (m_points_long.count(pt.first))
      {
        CHECK_AND_ASSERT_MES(pt.second == m_points_long[pt.first], false, "Longhash checkpoint at given height already exists, and hash for new checkpoint was different!");
      }
    }
    return true;
  }
}
