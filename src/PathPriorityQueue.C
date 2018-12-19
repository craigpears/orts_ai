#include "PathPriorityQueue.H"

using namespace std;

void PathPriorityQueue::InsertNode(Path& node)
{
	sint4 cost = node.GetCost();
	
	if(myList.empty())
	myList.push_back(node);
	else
	{
		list<Path>::iterator it;
		it = myList.begin();
		uint4 i = 0;
		while(it->GetCost() < cost && i < myList.size()){it++;i++;}
		myList.insert(it,node);	
	}
}

Path PathPriorityQueue::front()
{
	Path temp = myList.front();
	myList.pop_front();
	return temp;
}
