#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

class PcapWriter {
public:
  PcapWriter(const std::string& file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile(file, std::ios::out, PcapHelper::DLT_PPP);
  }

  void
  TracePacket(Ptr<const Packet> packet)
  {
    static PppHeader pppHeader;
    pppHeader.SetProtocol(0x0077);

    m_pcap->Write(Simulator::Now(), pppHeader, packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};


int
main(int argc, char* argv[])
{
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
  CommandLine cmd;
  cmd.Parse(argc, argv);

  NodeContainer nc1,nc2,nc3,nc4,nc5,nc6;
  nc1.Create(2);
  nc2.Add(nc1.Get(1));
  nc2.Create(1);
  nc3.Add(nc1.Get(1));
  nc3.Create(1);
  nc4.Add(nc2.Get(1));
  nc4.Create(1);
  nc5.Add(nc3.Get(1));
  nc5.Add(nc4.Get(1));
  nc6.Add(nc5.Get(1));
  nc6.Create(1);

  PointToPointHelper pointToPoint1, pointToPoint2; 
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("1ms"));

  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("1ms"));

  pointToPoint1.Install (nc1);
  pointToPoint2.Install (nc2);
  pointToPoint2.Install (nc3);
  pointToPoint2.Install (nc4);
  pointToPoint2.Install (nc5);
  pointToPoint1.Install (nc6);
  
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1");
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  /*ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();*/

  Ptr<Node> consumer = nc1.Get(0);

  Ptr<Node> producer = nc6.Get(1);

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("Frequency", StringValue("200")); // 96 interests a second

  consumerHelper.SetPrefix("/data");
  //consumerHelper.Install(consumer);
  ApplicationContainer consumerapp = consumerHelper.Install(consumer);
  consumerapp.Start(Seconds(0));
  consumerapp.Stop(Seconds(20));

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1280"));

  //ndnGlobalRoutingHelper.AddOrigins("/dst1", producer);
  producerHelper.SetPrefix("/data");
  ApplicationContainer producerapp = producerHelper.Install(producer);
  
  // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateRoutes();

  ndn::FibHelper::AddRoute(nc1.Get(0), "/data", nc1.Get(1), 1); // link to n1
  ndn::FibHelper::AddRoute(nc2.Get(0), "/data", nc2.Get(1), 1); // link to n1
  ndn::FibHelper::AddRoute(nc3.Get(0), "/data", nc3.Get(1), 1); // link to n1
  ndn::FibHelper::AddRoute(nc4.Get(0), "/data", nc4.Get(1), 1); // link to n1
  ndn::FibHelper::AddRoute(nc5.Get(0), "/data", nc5.Get(1), 1);  // link to n2
  ndn::FibHelper::AddRoute(nc6.Get(0), "/data", nc6.Get(1), 1); // link to n12

  PcapWriter trace("ndn-simple-trace.pcap");
  pointToPoint1.EnablePcapAll ("ndn1-6node");
  pointToPoint2.EnablePcapAll ("ndn2-6node");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeCallback(&PcapWriter::TracePacket, &trace));

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
