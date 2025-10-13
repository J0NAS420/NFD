#include "reservation-table.hpp"

namespace nfd {

std::vector<uint8_t> ReservationTable::cbsTrafficClasses = {1, 2};
std::map<std::string, std::string> ReservationTable::interfaceMap = {{"enp3s0", "enp3s0"}};
std::map< uint8_t, uint8_t > ReservationTable::priorityTrafficClassMap = {{3, 1}, {2, 2}};
srInfo ReservationTable::baselineSRConfig = {1250+14+4+80+24+100, 2816};
genInfo ReservationTable::baselineGenInfo = {1542, 1000000};

ReservationTable::ReservationTable()
{
  m_duplicateCheckMap = {};
  m_reservationMap = {};
}

void
ReservationTable::addReservationToMap(const Interest& interest, const std::string interface)
{
  uint8_t priority = (uint8_t) std::stoi(interest.getName().getSubName(1, 1).toUri().substr(5));
  if (priorityTrafficClassMap.find(priority) == priorityTrafficClassMap.end()) 
    return;

  if (m_duplicateCheckMap.find(interface) == m_duplicateCheckMap.end()) { // interface not found -> add new set/map to maps
    // add set to check for future duplicate interests
    std::set<std::string> interestNameSet = {};
    interestNameSet.insert(interest.getName().toUri());
    m_duplicateCheckMap.insert({interface, interestNameSet});
        
    // add map to count reserved bandwidth
    uint8_t trafficClass = priorityTrafficClassMap.at(priority);
    std::map<uint8_t, uint32_t> tcReservationMap = {};
    tcReservationMap.insert({trafficClass, interest.getTestValue().value()});
    m_reservationMap.insert({interface, tcReservationMap});
  }
  else { // interface found -> check if reservation is no duplicate -> if not add reservation
    std::set<std::string> interestNameSet = m_duplicateCheckMap.at(interface);
    if (interestNameSet.find(interest.getName().toUri()) == interestNameSet.end()) { // no duplicate -> add reservation
      m_duplicateCheckMap.at(interface).insert(interest.getName().toUri());
          
      uint8_t trafficClass = priorityTrafficClassMap.at(priority);
      if (m_reservationMap.at(interface).find(trafficClass) == m_reservationMap.at(interface).end())
        m_reservationMap.at(interface).insert({trafficClass, 0});
      m_reservationMap.at(interface).at(trafficClass) += interest.getTestValue().value(); // @todo Check if the value can be changed this way!
    }
  }
}

// Reserve bandwidth for an interface potentially different to ingress -> saved in interfaceMap map 
void
ReservationTable::addReservationIncoming(const Interest& interest, const FaceEndpoint& ingress)
{
  if (!interest.hasTestValue() || ingress.face.getLocalUri().getScheme().compare("dev") != 0) 
    return;

  const std::string ingressInterface = ingress.face.getLocalUri().getPath(); 
  if (interfaceMap.find(ingressInterface) == interfaceMap.end())
    return;
  const std::string dataInterface = interfaceMap.at(ingressInterface);

  addReservationToMap(interest, dataInterface);
}

// Reserve bandwidth for egress interface
void
ReservationTable::addReservationOutgoing(const Interest& interest, const Face& egress)
{
  if (!interest.hasTestValue() || egress.getLocalUri().getScheme().compare("dev") != 0) 
    return;

  const std::string dataInterface = egress.getLocalUri().getPath();

  addReservationToMap(interest, dataInterface);
}

void
ReservationTable::changeQdiscWithTimer()
{
  if (interfaceMap.empty() || priorityTrafficClassMap.empty())
    return;

  auto nowTime = std::chrono::steady_clock::now();
  if ( (std::chrono::duration<double, std::milli>(nowTime - m_lastQdiscChange).count()) < 2000) 
    return;
  
  m_lastQdiscChange = std::chrono::steady_clock::now();
  for (std::map< std::string, std::set<std::string> >::const_iterator dev = m_duplicateCheckMap.begin(); dev != m_duplicateCheckMap.end(); ++dev) {
    std::vector<srInfo> srInfoVector = {};
    for (std::vector<uint8_t>::const_iterator tc = cbsTrafficClasses.begin(); tc != cbsTrafficClasses.end(); ++tc) { // get reservations for each traffic class
      srInfo srInfoStruct = {};
      srInfoStruct.maxFrameSize = baselineSRConfig.maxFrameSize;
      if (m_reservationMap.at(dev->first).find(*tc) != m_reservationMap.at(dev->first).end()) { // reservations exist
        srInfoStruct.assignedBitrate = baselineSRConfig.assignedBitrate + m_reservationMap.at(dev->first).at(*tc);
        m_reservationMap.at(dev->first).at(*tc) = 0; // reset reservations
      } 
      else  // no reservations -> only use baseline config
        srInfoStruct.assignedBitrate = baselineSRConfig.assignedBitrate;
      srInfoVector.push_back(srInfoStruct);
    }
      
    std::vector<cbsConfigs> cbsConfigsVector = prepareCBSInfo(srInfoVector, baselineGenInfo);
    for (size_t i = 0; i < cbsTrafficClasses.size(); ++i) { // change CBS parameters for each traffic class
      std::string parentClassID = "100:" + std::to_string(cbsTrafficClasses.at(i));
      int qdiscError = change_cbs(dev->first.c_str(), "none", parentClassID.c_str(), cbsConfigsVector.at(i).hiCredit, 
          cbsConfigsVector.at(i).loCredit, cbsConfigsVector.at(i).idleSlope, cbsConfigsVector.at(i).loCredit);
      if (qdiscError)
        std::cerr << "Error with changing qdisc!" << std::endl;
    }
    m_duplicateCheckMap.at(dev->first).clear(); // reset duplicate checks 
  }
}

void
ReservationTable::configureJSON(std::string jsonString)
{
  return;
}

}