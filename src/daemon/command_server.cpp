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

#include "cryptonote_config.h"
#include "version.h"
#include "daemon/command_server.h"

namespace daemonize {

namespace p = std::placeholders;

t_command_server::t_command_server(
    uint32_t ip
  , uint16_t port
  )
  : m_parser(ip, port)
  , m_command_lookup()
{
  m_command_lookup.set_handler(
      "help"
    , std::bind(&t_command_server::help, this, p::_1)
    , "Show this help"
    );
  m_command_lookup.set_handler(
      "print_height"
    , std::bind(&t_command_parser_executor::print_height, &m_parser, p::_1)
    , "Print local blockchain height"
    );
  m_command_lookup.set_handler(
      "print_pl"
    , std::bind(&t_command_parser_executor::print_peer_list, &m_parser, p::_1)
    , "Print peer list"
    );
  m_command_lookup.set_handler(
      "print_cn"
    , std::bind(&t_command_parser_executor::print_connections, &m_parser, p::_1)
    , "Print connections"
    );
  m_command_lookup.set_handler(
      "print_bc"
    , std::bind(&t_command_parser_executor::print_blockchain_info, &m_parser, p::_1)
    , "Print blockchain info in a given blocks range, print_bc <begin_height> [<end_height>]"
    );
  m_command_lookup.set_handler(
      "print_block"
    , std::bind(&t_command_parser_executor::print_block, &m_parser, p::_1)
    , "Print block, print_block <block_hash> | <block_height>"
    );
  m_command_lookup.set_handler(
      "print_tx"
    , std::bind(&t_command_parser_executor::print_transaction, &m_parser, p::_1)
    , "Print transaction, print_tx <transaction_hash>"
    );
  m_command_lookup.set_handler(
      "start_mining"
    , std::bind(&t_command_parser_executor::start_mining, &m_parser, p::_1)
    , "Start mining for specified address, start_mining <addr> [threads=1]"
    );
  m_command_lookup.set_handler(
      "stop_mining"
    , std::bind(&t_command_parser_executor::stop_mining, &m_parser, p::_1)
    , "Stop mining"
    );
  m_command_lookup.set_handler(
      "print_pool"
    , std::bind(&t_command_parser_executor::print_transaction_pool_long, &m_parser, p::_1)
    , "Print transaction pool (long format)"
    );
  m_command_lookup.set_handler(
      "print_pool_sh"
    , std::bind(&t_command_parser_executor::print_transaction_pool_short, &m_parser, p::_1)
    , "Print transaction pool (short format)"
    );
  m_command_lookup.set_handler(
      "show_hr"
    , std::bind(&t_command_parser_executor::show_hash_rate, &m_parser, p::_1)
    , "Start showing hash rate"
    );
  m_command_lookup.set_handler(
      "hide_hr"
    , std::bind(&t_command_parser_executor::hide_hash_rate, &m_parser, p::_1)
    , "Stop showing hash rate"
    );
  m_command_lookup.set_handler(
      "save"
    , std::bind(&t_command_parser_executor::save_blockchain, &m_parser, p::_1)
    , "Save blockchain"
    );
  m_command_lookup.set_handler(
      "set_log"
    , std::bind(&t_command_parser_executor::set_log_level, &m_parser, p::_1)
    , "set_log <level> - Change current log detalization level, <level> is a number 0-4"
    );
  m_command_lookup.set_handler(
      "diff"
    , std::bind(&t_command_parser_executor::show_difficulty, &m_parser, p::_1)
    , "Show difficulty"
    );
  m_command_lookup.set_handler(
      "stop_daemon"
    , std::bind(&t_command_parser_executor::stop_daemon, &m_parser, p::_1)
    , "Stop the daemon"
    );
  m_command_lookup.set_handler(
      "print_status"
    , std::bind(&t_command_parser_executor::print_status, &m_parser, p::_1)
    , "Prints daemon status"
    );
  m_command_lookup.set_handler(
      "limit"
    , std::bind(&t_command_parser_executor::set_limit, &m_parser, p::_1)
    , "limit <kB/s> - Set download and upload limit"
    );
  m_command_lookup.set_handler(
      "limit-up"
    , std::bind(&t_command_parser_executor::set_limit_up, &m_parser, p::_1)
    , "limit <kB/s> - Set upload limit"
    );
  m_command_lookup.set_handler(
      "limit-down"
    , std::bind(&t_command_parser_executor::set_limit_down, &m_parser, p::_1)
    , "limit <kB/s> - Set download limit"
    );
}

bool t_command_server::process_command_str(const std::string& cmd)
{
  return m_command_lookup.process_command_str(cmd);
}

bool t_command_server::process_command_vec(const std::vector<std::string>& cmd)
{
  return m_command_lookup.process_command_vec(cmd);
}

bool t_command_server::help(const std::vector<std::string>& args)
{
  std::cout << get_commands_str() << std::endl;
  return true;
}

std::string t_command_server::get_commands_str()
{
  std::stringstream ss;
  ss << CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << std::endl;
  ss << "Commands: " << std::endl;
  std::string usage = m_command_lookup.get_usage();
  boost::replace_all(usage, "\n", "\n  ");
  usage.insert(0, "  ");
  ss << usage << std::endl;
  return ss.str();
}

} // namespace daemonize
