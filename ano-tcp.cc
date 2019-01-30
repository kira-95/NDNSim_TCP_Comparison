#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OurScript");

int main (int argc, char *argv[])
{
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
  Time::SetResolution (Time::NS);
  bool verbose = true;
  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.Parse (argc,argv);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);

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

  NetDeviceContainer d1,d2,d3,d4,d5,d6;
  d1 = pointToPoint1.Install (nc1);
  d2 = pointToPoint2.Install (nc2);
  d3 = pointToPoint2.Install (nc3);
  d4 = pointToPoint2.Install (nc4);
  d5 = pointToPoint2.Install (nc5);
  d6 = pointToPoint1.Install (nc6);
  
  InternetStackHelper stack;
  stack.Install (nc1);
  stack.Install (nc6);
  stack.Install (nc2.Get(1));
  stack.Install (nc3.Get(1));

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i1 = address.Assign (d1);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = address.Assign (d2);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i3 = address.Assign (d3);
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i4 = address.Assign (d4);
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i5 = address.Assign (d5);
  address.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i6 = address.Assign (d6);


  //sink for reciever????
  PacketSinkHelper sink ("ns3::TcpSocketFactory",Address(InetSocketAddress (Ipv4Address::GetAny (), 10)));
  //set a node as reciever
  ApplicationContainer app = sink.Install (nc1.Get(0));

  app.Start (Seconds (0.0));
  app.Stop (Seconds (20.0));

  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address(InetSocketAddress (Ipv4Address ("10.1.1.1"), 10)));
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));  

  onOffHelper.SetAttribute ("DataRate",StringValue ("1.9Mbps"));
  onOffHelper.SetAttribute ("PacketSize",UintegerValue(1280));
  // ApplicationContainer
  app = onOffHelper.Install (nc6.Get(1));
  // Start the application
  app.Start (Seconds (0.0));
  app.Stop (Seconds (20.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint1.EnablePcapAll ("tcp-6node");
  pointToPoint2.EnablePcapAll ("tcp-6node");
  Simulator::Run ();
}

