#pragma once

#include <asio.hpp>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <set>

class Peers {
public:
  using ips_t = std::set<ip_t>;

  bool add(const ip_t& IP, const std::string& key) {
    std::lock_guard<std::mutex> lock(peer_lock_);
    auto& this_sender_ips = keyToIp_[key];
    if(this_sender_ips.find(IP) == this_sender_ips.end()) {
      this_sender_ips.insert(IP);
      return true;
    }
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(peer_lock_);
    return keyToIp_.empty();
  }

  ips_t getIpByPubKey(const std::string& pub_key) const {
    std::lock_guard<std::mutex> lock(peer_lock_);
    auto ip = keyToIp_.find(pub_key);
    if(ip!=keyToIp_.end()) {
      return ip->second;
    }
    throw std::runtime_error("Peer unknown");
  }

  ips_t getAllIPs() const {
    std::lock_guard<std::mutex> lock(peer_lock_);
    ips_t return_set;
    for(const auto& ips : keyToIp_) {
      return_set.insert(ips.second.begin(), ips.second.end());
    }
    return return_set;
  }

protected:
  mutable std::mutex peer_lock_;
  std::unordered_map<std::string, ips_t > keyToIp_;
};
