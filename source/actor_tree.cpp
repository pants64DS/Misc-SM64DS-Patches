#include "actor_tree.h"
#include <ranges>

using Node = ActorTreeNode;

constinit Node* Node::root;

[[gnu::target("thumb")]]
Node* Node::Rotate(Node* Node::* from, Node* Node::* to)
{
	Node* const newParent = this->*to;
	this->*to = std::exchange(newParent->*from, this);

	this->UpdateHeight();
	newParent->UpdateHeight();

	return newParent;
}

[[gnu::target("thumb")]]
void Node::Insert(Node*& parent, Node& newNode)
{
	if (parent == nullptr)
	{
		parent = &newNode;
		return;
	}

	Insert(parent->right, newNode);

	parent->UpdateHeight();

	if (parent->GetHeightDiff() < -1)
	{
		if (newNode.uniqueID < parent->right->uniqueID)
			RotateRight(parent->right);
		
		RotateLeft(parent);
	}
}

[[gnu::target("thumb")]]
void Node::Remove(Node*& target, unsigned uniqueID)
{
	if (uniqueID < target->uniqueID)
		Remove(target->left, uniqueID);

	else if (uniqueID > target->uniqueID)
		Remove(target->right, uniqueID);
	
	else if (!target->left && !target->right) // if target is a leaf node
		target = nullptr;
	
	else if (target->right && !target->left) // if target only has the right child
		target = target->right;

	else if (target->left && !target->right) // if target only has the left child
		target = target->left;

	else if (target->left && target->right) // if target has both children
	{
		unsigned numParents = 0;
		Node*& successor = [&]
		{
			auto s = std::ref(target->right);
			while (s.get()->left)
			{
				s = s.get()->left;
				numParents++;
			}
			return s;
		}();

		Node* const newSuccessor = successor->right;
		successor->left  = target->left;

		if (successor != target->right)
			successor->right = target->right;

		target = successor;
		successor = newSuccessor;

		Node** parentArray[numParents];
		std::ranges::subrange parents(&parentArray[0], &parentArray[numParents]);

		for (auto lastParent = std::ref(target->right); Node**& parent : parents)
		{
			parent = &lastParent.get();
			lastParent = lastParent.get()->left;
		}

		for (Node** parent : std::ranges::reverse_view(parents))
			RestoreBalance(*parent);
	}

	RestoreBalance(target);
}

[[gnu::target("thumb")]]
void Node::RestoreBalance(Node*& node)
{
	if (node == nullptr) return;

	node->UpdateHeight();

	const int heightDiff = node->GetHeightDiff();
	
	if (heightDiff > 1)
	{
		if (node->left && node->left->GetHeightDiff() < 0)
			RotateLeft(node->left);
		
		RotateRight(node);
	}
	else if (heightDiff < -1)
	{
		if (node->right && node->right->GetHeightDiff() > 0)
			RotateRight(node->right);
		
		RotateLeft(node);
	}
}

Actor* Node::Find(unsigned uniqueID)
{
	Node* node = root;

	while (node)
	{
		if (uniqueID < node->uniqueID)
			node = node->left;
		else if (uniqueID > node->uniqueID)
			node = node->right;
		else
			return &node->GetActor();
	}

	return nullptr;
}

asm("nsub_02010f3c = _ZN13ActorTreeNode4FindEj");
