
#include "ns3/tag.h"

#include "ns3/packet.h"

#include "ns3/uinteger.h"

#include "vector"

#include <iostream>


using namespace ns3;
using namespace std;


// define this class in a public header

class MyTag : public Tag

{

public:

  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;

  virtual void Serialize (TagBuffer i) const;

  virtual void Deserialize (TagBuffer i);

  virtual void Print (std::ostream &os) const;

  
  // these are our accessors to our tag structure

  void SetSimpleValue (uint8_t value);

  uint8_t GetSimpleValue (void) const;

  void SetSourceAddress (uint8_t value);

  uint8_t GetSourceAddress (void) const;

 void SetDestAddress (uint8_t value);

  uint8_t GetDestAddress (void) const;
  
  
  // new methods
  
  void SetPacketType (uint8_t value);

  uint8_t GetPacketType (void) const;
  
  //void SetNodeList (vector<uint8_t> value);

  //vector<uint8_t> GetNodeList (void) const;

private:

  uint8_t m_simpleValue;
 uint8_t m_srcaddr;
 uint8_t m_destaddr;
 vector<uint8_t> nodeList;
 uint8_t packetType;

};


  void MyTag::SetSourceAddress (uint8_t value)
       {
        
        m_srcaddr = value;
}

  uint8_t MyTag::GetSourceAddress (void) const
{
        return m_srcaddr;
}

 void MyTag::SetDestAddress (uint8_t value)
{
             m_destaddr = value;
}

 uint8_t MyTag::GetDestAddress (void) const
{
         return m_destaddr;
}


TypeId 

MyTag::GetTypeId (void)

{

  static TypeId tid = TypeId ("ns3::MyTag")

    .SetParent<Tag> ()

    .AddConstructor<MyTag> ()

    .AddAttribute ("SimpleValue",

                   "A simple value",

                   EmptyAttributeValue (),

                   MakeUintegerAccessor (&MyTag::GetSimpleValue),

                   MakeUintegerChecker<uint8_t> ())

    ;

/*
tid.AddAttribute ("Src Index",
                   "A simple index",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&MyTag::GetSourceAddress),
                   MakeUintegerChecker<uint8_t> ())
    ;
tid.AddAttribute ("Dest Index",
                   "A simple index",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&MyTag::GetDestAddress),
                   MakeUintegerChecker<uint8_t> ())
    ;
*/

  return tid;

}

TypeId 

MyTag::GetInstanceTypeId (void) const

{

  return GetTypeId ();

}

uint32_t 
MyTag::GetSerializedSize (void) const

{

  return 4;

}

void 

MyTag::Serialize (TagBuffer i) const

{

  i.WriteU8 (m_simpleValue);
i.WriteU8 (m_srcaddr);
i.WriteU8 (m_destaddr);
i.WriteU8 (packetType);
//i.Write (nodeList, nodeList.size());

}

void 

MyTag::Deserialize (TagBuffer i)

{

  m_simpleValue = i.ReadU8 ();
m_srcaddr = i.ReadU8 ();
m_destaddr = i.ReadU8 ();
packetType = i.ReadU8 ();
//nodeList

}

void 

MyTag::Print (std::ostream &os) const

{

  os << "v=" << (uint32_t)m_simpleValue;
 os << "s=" << (uint32_t)m_srcaddr;
 os << "d=" << (uint32_t)m_destaddr;

}

void 

MyTag::SetSimpleValue (uint8_t value)

{

  m_simpleValue = value;

}

uint8_t 
MyTag::GetSimpleValue (void) const

{

  return m_simpleValue;

}

void MyTag::SetPacketType (uint8_t value){
	
	packetType = value;
}

  uint8_t MyTag::GetPacketType (void) const{
	  
	  return packetType;
  }
  
  /*
  
  void MyTag::SetNodeList (vector<uint8_t> value){
	  
	  nodeList.assign (value.begin(),value.end());
  }

  vector<uint8_t> MyTag::GetNodeList (void) const{
	  
	  return nodeList;
  }
*/
