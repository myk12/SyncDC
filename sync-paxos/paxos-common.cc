#include "paxos-common.h"

Proposal::Proposal()
    : m_proposalId(0), m_nodeId(0), m_value(0), m_numAck(0) {}

Proposal::Proposal(uint64_t proposalId, uint32_t serverId, ns3::Time proposeTime, ns3::Time acceptTime)
    : m_proposalId(proposalId), m_nodeId(serverId), m_proposeTime(proposeTime), m_acceptTime(acceptTime), m_value(0), m_numAck(0) {}

Proposal::~Proposal() {
    // Destructor logic if needed
}

bool
Proposal::operator>(const Proposal& other) const {
    return m_proposalId > other.m_proposalId;
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

void Proposal::setProposerId(uint32_t proposerId) {
    m_proposerId = proposerId;
}

uint32_t Proposal::getProposerId() {
    return m_proposerId;
}

void Proposal::setCreateTime(ns3::Time createTime) {
    m_createTime = createTime;
}

ns3::Time Proposal::getCreateTime() {
    return m_createTime;
}

void Proposal::setReceiveTime(ns3::Time receiveTime) {
    m_receiveTime = receiveTime;
}

ns3::Time Proposal::getReceiveTime() {
    return m_receiveTime;
}

void Proposal::setDecisionTime(ns3::Time decisionTime) {
    m_decisionTime = decisionTime;
}

ns3::Time Proposal::getDecisionTime() {
    return m_decisionTime;
}

void Proposal::setPropState(PropState propState) {
    m_propState = propState;
}

Proposal::PropState
Proposal::getPropState() {
    return m_propState;
}

void
Proposal::setNumDecisionAck(uint32_t numDecisionAck) {
    m_numDecisionAck = numDecisionAck;
}

uint32_t
Proposal::getNumDecisionAck() {
    return m_numDecisionAck;
}

void
Proposal::incrementNumDecisionAck() {
    m_numDecisionAck++;
}

void
Proposal::setDecisionAckTime(ns3::Time decisionAckTime) {
    m_decisionAckTime = decisionAckTime;
}

ns3::Time
Proposal::getDecisionAckTime() {
    return m_decisionAckTime;
}