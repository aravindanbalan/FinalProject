
#include "ns3/core-module.h"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <ns3/node.h>
using namespace ns3;
using namespace std;

class Cluster {

	private: 
		int clusterID;
		int topic;
		int numNodes;
		vector<Ptr<Node> > masterNodes;
		vector<Ptr<Node> > nodeList;
		vector<Ptr<Node> > slaveNodes;
	
	public:
	    
	    Cluster(int ID, int top)
	    {
			clusterID = ID;
			numNodes = 0;
			topic = top;
		}
		vector<Ptr<Node> > getMasters();
		vector< Ptr<Node> > getSlaveNodes();
		bool isMaster(Ptr<Node> node);
		vector < Ptr<Node> > getNodeList();
		void addNodeToCluster(Ptr<Node> node);
		void removeNodeFromCluster(Ptr<Node> node);
		int getNumNodes();
		int getNumSlaveNodes();
		int getTopic();	
		void setSlaveNodes(vector<Ptr<Node> > slaveList);
		void setMaster(vector<Ptr<Node> > master);
};

	
int Cluster::getNumNodes()
{
		return numNodes;
}
int Cluster::getTopic()
{
		return topic;
}
void Cluster::addNodeToCluster(Ptr<Node> node){
	
		nodeList.push_back(node);
		numNodes++;	
}

void Cluster::removeNodeFromCluster(Ptr<Node> node){
	
	vector <Ptr<Node> > nodeList = getNodeList();
	for( vector<Ptr<Node> >::iterator iter = nodeList.begin(); iter != nodeList.end(); ++iter )
	{
		if( *iter == node )
		{
			nodeList.erase( iter );
			numNodes--;
			break;
		}
	}
	
}

vector<Ptr<Node> > Cluster::getMasters(){
	
		return masterNodes;
}
bool Cluster::isMaster(Ptr<Node> node)
{

		for(uint32_t i =0;i< masterNodes.size();i++)
		{
			if(masterNodes[i] == node)
				return true;
		}
			
	return false;
}

vector < Ptr<Node> > Cluster::getNodeList(){
	
	return nodeList;
}
/*
vector< Ptr<Node> > Cluster::getSlaveNodes(){

cout<<"Slave string111 ....."<<endl;
	vector < Ptr<Node> > slaveList = getNodeList();
	vector<Ptr<Node> > master = getMasters();
		for(vector<Ptr<Node> >::iterator it = slaveList.begin(); it != slaveList.end(); it++)
		{
		  if(std::find(master.begin(), master.end(), *it) != master.end())
		  {
			 
			slaveList.erase(it);
			if(master.size() == 1) break;
		  }
		}
		cout<<"Slave string22 ....."<<endl;
		return slaveList;
}
* */

vector< Ptr<Node> > Cluster::getSlaveNodes(){

		return slaveNodes;
}

/*
void Cluster::setSlaveNodes(){
	
	cout<<"Slave string111 ....."<<endl;
	vector < Ptr<Node> > slaveList = getNodeList();
	vector<Ptr<Node> > master = getMasters();
	cout<<"Slave string111 ...sdfdsadfd.. "<<slaveList.size()<<endl;
	cout<<"Slave string111 ...sdfdsadfd.. "<<master.size()<<endl;
		for(vector<Ptr<Node> >::iterator it = slaveList.begin(); it != slaveList.end(); it++)
		{
		  if(std::find(master.begin(), master.end(), *it) != master.end())
		  {
			 
			slaveList.erase(it);
			if(master.size() == 1) break;
		  }
		}
		slaveNodes = slaveList;
		cout<<"Slave string22 ....."<<endl;
}

*/

void Cluster::setSlaveNodes(vector<Ptr<Node> > slaveList){
	slaveNodes = slaveList;
}


	void Cluster::setMaster(vector<Ptr<Node> > masters){
		
		masterNodes = masters;
	}

int Cluster::getNumSlaveNodes(){
	
	return getSlaveNodes().size();
}
