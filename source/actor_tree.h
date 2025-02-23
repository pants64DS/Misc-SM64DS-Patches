#ifndef ACTOR_TREE_INCLUDED
#define ACTOR_TREE_INCLUDED

#include <algorithm>
#include "SM64DS_PI.h"

class ActorTreeNode
{
	const unsigned uniqueID;
	Actor& actor;
	unsigned height = 1;
	ActorTreeNode* left = nullptr;
	ActorTreeNode* right = nullptr;

	ActorTreeNode(const ActorTreeNode&) = delete;
	ActorTreeNode(ActorTreeNode&&) = delete;
	ActorTreeNode& operator=(const ActorTreeNode&) = delete;
	ActorTreeNode& operator=(ActorTreeNode&&) = delete;

	[[gnu::target("thumb")]]
	static unsigned Getheight(const ActorTreeNode* node)
	{
		if (node)
			return node->height;
		else
			return 0;
	}

	[[gnu::target("thumb")]]
	void UpdateHeight()
	{
		height = std::max(Getheight(left), Getheight(right)) + 1;
	}

	[[gnu::target("thumb")]]
	int GetHeightDiff() const
	{
		return Getheight(left) - Getheight(right);
	}

	ActorTreeNode* Rotate(ActorTreeNode* ActorTreeNode::* from, ActorTreeNode* ActorTreeNode::* to);

	[[gnu::target("thumb")]]
	static void RotateLeft(ActorTreeNode*& pivot)
	{
		pivot = pivot->Rotate(&ActorTreeNode::left, &ActorTreeNode::right);
	}

	[[gnu::target("thumb")]]
	static void RotateRight(ActorTreeNode*& pivot)
	{
		pivot = pivot->Rotate(&ActorTreeNode::right, &ActorTreeNode::left);
	}

	static void Insert(ActorTreeNode*& root, ActorTreeNode& newNode);
	static void Remove(ActorTreeNode*& root, unsigned uniqueID);
	static void RestoreBalance(ActorTreeNode*& node);	

	static ActorTreeNode* root;

public:
	[[gnu::target("thumb")]]
	ActorTreeNode(Actor& actor) : uniqueID(actor.uniqueID), actor(actor) { Insert(root, *this); }
	[[gnu::target("thumb")]]
	~ActorTreeNode() { Remove(root, uniqueID); }

	static Actor* Find(unsigned uniqueID);

	      Actor& GetActor()       { return actor; }
	const Actor& GetActor() const { return actor; }
};

#endif