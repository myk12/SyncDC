#include "paxos-common.h"

Proposal::Proposal()
    : m_proposalId(0), m_nodeId(0), m_value(0), m_numAck(0) {}

Proposal::Proposal(uint64_t proposalId, uint32_t nodeId, ns3::Time proposeTime, ns3::Time acceptTime)
    : m_proposalId(proposalId), m_nodeId(nodeId), m_proposeTime(proposeTime), m_acceptTime(acceptTime), m_value(0), m_numAck(0) {}

Proposal::~Proposal() {
    // Destructor logic if needed
}

uint64_t Proposal::getProposalId() {
    return m_proposalId;
}

uint32_t Proposal::getNodeId() {
    return m_nodeId;
}

void Proposal::setProposalId(uint64_t proposalId) {
    m_proposalId = proposalId;
}

void Proposal::setNodeId(uint32_t nodeId) {
    m_nodeId = nodeId;
}

ns3::Time Proposal::getProposeTime() {
    return m_proposeTime;
}

ns3::Time Proposal::getAcceptTime() {
    return m_acceptTime;
}

void Proposal::setProposeTime(ns3::Time proposeTime) {
    m_proposeTime = proposeTime;
}

void Proposal::setAcceptTime(ns3::Time acceptTime) {
    m_acceptTime = acceptTime;
}

void Proposal::setValue(uint32_t value) {
    m_value = value;
}

uint32_t Proposal::getValue() {
    return m_value;
}

void Proposal::setNumAck(uint32_t numAck) {
    m_numAck = numAck;
}   

uint32_t Proposal::getNumAck() {
    return m_numAck;
}

void Proposal::incrementNumAck() {
    m_numAck++;
}


bool isProposalSignature(uint8_t *data) {
    return (data[0] == 'P' && data[1] == 'R' && data[2] == 'O' && data[3] == 'P');
}

bool isAcceptSignature(uint8_t *data) {
    return (data[0] == 'A' && data[1] == 'C' && data[2] == 'C' && data[3] == 'E');
}

bool isDecisionSignature(uint8_t *data) {
    return (data[0] == 'D' && data[1] == 'E' && data[2] == 'C' && data[3] == 'I');
}

// Implementation of Proposal class methods

// Set log formatter

// Self defined log format
void MyLogFormatter (std::ostream &os, ns3::Ptr<const ns3::Object> object, const ns3::LogLevel logLevel, const char *file, uint32_t line, const char *cond, const std::string &msg)
{
    // Get current time
    ns3::Time now = ns3::Simulator::Now();
    uint32_t objectId = 0;

    os << "[" << now.GetMicroSeconds() << "us]";
    // If object is null, you can print a special value or not print anything
    if (objectId != 0)
    {
        os << "[Node " << objectId << "]";
    }
    else if (object != nullptr)
    {
        os << "[" << object->GetTypeId().GetName() << "]";
    }
    os << " " << msg << std::endl;
}
