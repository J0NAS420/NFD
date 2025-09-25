#include <iostream>
#include <fstream>
#include <string>
#include <bits/stdc++.h>
#include <chrono>

#include "face/face-endpoint.hpp"

#include "cbs_netlink.hpp"
#include "qdisc_info.hpp"


namespace nfd {

struct reservationValues {
  uint64_t testValue;
  Name interestName;
};

class ReservationTable
{
public:
  explicit
  ReservationTable();

  void
  addReservationIncoming(const Interest& interest, const FaceEndpoint& ingress);

  void
  addReservationOutgoing(const Interest& interest, const Face& egress);

  void
  changeQdiscWithTimer();

  static void
  configureJSON(std::string jsonString);

private:

  void
  addReservationToMap(const Interest& interest, const std::string interface);

private:
  std::chrono::time_point<std::chrono::steady_clock> m_lastQdiscChange;

  // map interfaces to interest names 
  std::map< std::string, std::set<std::string> > m_duplicateCheckMap;
  // map interfaces to map of traffic class to accumulated reserved bandwidth
  std::map< std::string, std::map<uint8_t, uint32_t> > m_reservationMap;

  // vector of traffic classes in descending order that have a CBS qdisc installed
  static std::vector<uint8_t> cbsTrafficClasses;

  // Maps the incoming interest interfaces to interfaces where data is received
  static std::map<std::string, std::string> interfaceMap;

  // Map the packet priorities to MQPRIO/TAPRIO traffic classes of CBS qdiscs 
  // if priority is not present, then qdisc is not CBS!
  static std::map< uint8_t, uint8_t > priorityTrafficClassMap;

  static srInfo baselineSRConfig;
  static genInfo baselineGenInfo;

};

} // namespace nfd