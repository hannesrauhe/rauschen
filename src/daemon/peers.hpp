#pragma once

#include "common.hpp"
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
    return false;
  }

  bool remove(const ip_t& IP) {
    std::lock_guard<std::mutex> lock(peer_lock_);
    std::vector< std::string > keys_removed;
    for(auto& ips : keyToIp_) {
      if(ips.second.erase(IP)>=1 && ips.second.empty()) {
        keys_removed.push_back( ips.first );
      }
    }
    if(keys_removed.empty())
      return false;
    for(const auto& key_removed : keys_removed) {
      keyToIp_.erase(key_removed);
    }
    return true;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(peer_lock_);
    return keyToIp_.empty();
  }

  bool isPeer(const ip_t& IP) const {
    std::lock_guard<std::mutex> lock(peer_lock_);
    for(const auto& ips : keyToIp_) {
      if(ips.second.find(IP)!=ips.second.end()) {
        return true;
      }
    }
    return false;
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

  std::vector<std::string> getAllKeys() const {
    std::lock_guard<std::mutex> lock(peer_lock_);
    std::vector<std::string> return_vec(keyToIp_.size());
    size_t i = 0;
    for(const auto& ips : keyToIp_) {
      return_vec[i++] = ips.first;
    }
    return return_vec;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(peer_lock_);
    return keyToIp_.size();
  }


protected:
  mutable std::mutex peer_lock_;
  std::unordered_map<std::string, ips_t > keyToIp_;
};
