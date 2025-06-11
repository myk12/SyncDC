#include "paxos-common.h"

Proposal::Proposal()
    : m_proposalId(0), m_nodeId(0), m_value(0), m_numAck(0) {}

Proposal::Proposal(uint64_t proposalId, uint32_t serverId, ns3::Time proposeTime, ns3::Time acceptTime)
    : m_proposalId(proposalId), m_nodeId(serverId), m_proposeTime(proposeTime), m_acceptTime(acceptTime), m_value(0), m_numAck(0) {}

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

void Proposal::setNodeId(uint32_t serverId) {
    m_nodeId = serverId;
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
