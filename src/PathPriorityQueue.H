#ifndef PathPriorityQueue_H
#define PathPriorityQueue_H

#include <list>
#include "Constants.H"
#include "Structures.H"

/*
A priority queue implemented using a linked list for paths
*/

class PathPriorityQueue
{
	public:
	void InsertNode(Path& node);
	Path front();//Retrieve and remove the first node
	std::list<Path> myList;
	private:
	
};

#endif
