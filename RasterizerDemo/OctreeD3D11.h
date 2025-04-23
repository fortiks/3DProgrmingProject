#pragma once
#include <algorithm> // for std::find
#include <vector>
#include <DirectXCollision.h>


#define MAX_ELEMENTS 2

#define MAX_DEPTH 4

template<typename T>
class OcTree
{
private:
	struct Node
	{
		std::vector<const T*> elements;
		DirectX::BoundingBox boundingBox;
		std::unique_ptr<Node> children[8];
	};

	std::unique_ptr<Node> root;

	
	void AddToNode(const T* elementAddress, std::unique_ptr<Node>& node, int depth);

	void CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node,
		std::vector<const T*>& foundObjects);

	bool HasObjects(Node* node);
	size_t GenerateGraphvizLinks(std::string& data, std::unique_ptr<Node>& node, size_t& nodeCounter);
	

	

public:
	OcTree() = default;
	~OcTree();
	void Initialize(size_t width, size_t height);

	void AddElement(const T* elementAddress);

	// culling
	
	std::vector<const T*> CheckTree(const DirectX::BoundingFrustum& frustum);
	

	
	std::string ToGraphviz();

};

template<typename T>
void OcTree<T>::AddToNode(const T* elementAddress,
	std::unique_ptr<Node>& node, int depth)
{
	bool collision = node->boundingBox.Intersects(elementAddress->boundingBox);

	if (!collision) // No collision found, node and potential children not relevan
	{
		return;
	}

	if (node->children[0] == nullptr) // if first child is null, all children are null
	{
		if (node->elements.size() < MAX_ELEMENTS || depth >= MAX_DEPTH)
		{
			node->elements.push_back(elementAddress);
			return;

		}
		else // The node is a leaf node, but there is no more room for elements
		{
			// Add child nodes to the node based on this node's covered volume
			

			DirectX::XMFLOAT3 corners[8][2];
			DirectX::XMFLOAT3 min = { node->boundingBox.Center.x - node->boundingBox.Extents.x,
						   node->boundingBox.Center.y - node->boundingBox.Extents.y,
						   node->boundingBox.Center.z - node->boundingBox.Extents.z };
			DirectX::XMFLOAT3 max = { node->boundingBox.Center.x + node->boundingBox.Extents.x,
						   node->boundingBox.Center.y + node->boundingBox.Extents.y,
						   node->boundingBox.Center.z + node->boundingBox.Extents.z };



			DirectX::XMFLOAT3 center = node->boundingBox.Center; // right now 0,0,-10
			DirectX::XMFLOAT3 extents = node->boundingBox.Extents;

			// === FrontSide ===
			// cube1  front left up cube
			corners[0][0] = {center.x + extents.x, center.y,
				center.z + extents.z}; // Bottom-center-left-front  (20,0, 0) 
			corners[0][1] = {center.x, center.y + extents.y, center.z}; // top-center-up-front (0,20, -10)
 
			// cube2 front left down cube
			corners[1][0] = {center.x + extents.x, center.y - extents.y, center.z + extents.z};//Bottom-left-front  (20,-20, 0) 
			corners[1][1] = center; // center

			// cube3 front right up cube
			// center.x, center.y, center.z
			corners[2][0] = {center.x, center.y, center.z + extents.z}; // right-center-front
			corners[2][1] = {center.x - extents.x, center.y + extents.y, center.z}; // top corner

			// cube4 front right down cube
			corners[3][0] = { center.x, center.y - extents.y, center.z + extents.z  }; // Top-left-back
			corners[3][1] = { center.x - extents.x, center.y, center.z }; // Top-right-back

			// === BackSide === 
			corners[4][0] = { center.x + extents.x, center.y,
				center.z}; // Bottom-center-left-front  (20,0, -10) 
			corners[4][1] = { center.x, center.y + extents.y, center.z - extents.z }; // top-center-up-front (0,20, -20)

			// cube2 front left down cube
			corners[5][0] = { center.x + extents.x, center.y - extents.y, center.z};//Bottom-left-front  (20,-20, -10) 
			corners[5][1] = { center.x, center.y, center.z - extents.z }; // (0,0, -20)

			// cube3 front right up cube
			// center.x, center.y, center.z
			corners[6][0] = { center.x, center.y, center.z }; // (0,0,-10)
			corners[6][1] = { center.x - extents.x, center.y + extents.y, center.z - extents.z}; // (-20, 20, -20)

			// cube4 front right down cube
			corners[7][0] = { center.x, center.y - extents.y, center.z }; // (0,-20, -10)
			corners[7][1] = { center.x - extents.x, center.y, center.z - extents.z }; // (-20, 0, -20)


			
			for (int i = 0; i < 8; i++)
			{
				node->children[i] = std::make_unique<Node>();
				//DirectX::XMVECTOR corner1 = DirectX::XMLoadFloat3(&corners[i]);
				//DirectX::XMVECTOR corner2 = DirectX::XMLoadFloat3(&corners[(i + 1) % 8]);
				DirectX::XMVECTOR corner1 = DirectX::XMLoadFloat3(&corners[i][0]);
				DirectX::XMVECTOR corner2 = DirectX::XMLoadFloat3(&corners[i][1]);
				DirectX::BoundingBox::CreateFromPoints(node->children[i]->boundingBox, corner1, corner2);
			}




			// For each of the currently stored elements in this node, attempt to add them to the new child nodes
			for (const T* storedElement : node->elements)
			{

				for (int i = 0; i < 8; ++i)
				{
					AddToNode(storedElement, node->children[i], depth + 1);
				}

			}
			node->elements.clear();
		}

	}
	if (depth < MAX_DEPTH)
	{
		for (int i = 0; i < 8; ++i) {
			if (node->children[i]) {
				AddToNode(elementAddress, node->children[i], depth + 1);
			}
		}
	}
	else
	{
		node->elements.push_back(elementAddress); // Store in this node if max depth is reached
	}
};

template<typename T>
void OcTree<T>::CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node,
	std::vector<const T*>& foundObjects)
{
	bool collision = frustum.Intersects(node->boundingBox);

	if (!collision) // No collision found, node and potential children not relevan
	{
		return;
	}

	if (node->children[0] == nullptr)
	{
		for (const T* storedElement : node->elements)
		{
			DirectX::BoundingBox worldBoundingBox;
			
			//storedElement->boundingBox.Transform(worldBoundingBox, storedElement->world); // Transfrom to current World Space
			collision = frustum.Intersects(storedElement->boundingBox);

			if (collision)
			{
				if (std::find(foundObjects.begin(), foundObjects.end(), storedElement) == foundObjects.end()) // O(n) 
				{
					foundObjects.push_back(storedElement);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 8; ++i) {
			if (node->children[i]) {
				CheckNode(frustum, node->children[i], foundObjects);
			}
		}
	}
};

template<typename T>
size_t OcTree<T>::GenerateGraphvizLinks(std::string& data, std::unique_ptr<Node>& node, size_t& nodeCounter)
{
	size_t myID = nodeCounter;
	data += std::to_string(myID) + "[label = \"Node: " + std::to_string(myID) + ' ';
	data += "\n Objects: " + std::to_string(node->elements.size()) + " ";
	data += "\"]\n";

	if (node->children[0] != nullptr) {
		for (size_t i = 0; i < 8; ++i)
		{
			if (node->children[i] && HasObjects(node->children[i].get())) {
				{
					std::string childID = std::to_string(GenerateGraphvizLinks(data, node->children[i], ++nodeCounter));
					data += std::to_string(myID) + " -> " + childID + '\n';
				}

			}
		}
	}
	return myID;

}

template<typename T>
inline bool OcTree<T>::HasObjects(Node* node)
{
	// If this node contains objects
	if (!node->elements.empty()) return true;

	// If this node has children, check if any of them contain objects
	if (node->children[0] != nullptr) {
		for (size_t i = 0; i < 8; ++i) {
			if (node->children[i] && HasObjects(node->children[i].get())) {
				return true;
			}
		}
	}

	return false; // No objects found in this node or its descendants
}

template<typename T>
inline OcTree<T>::~OcTree()
{
	
}

template<typename T>
inline void OcTree<T>::Initialize(size_t width, size_t height)
{
	
	root = std::make_unique<Node>();
	DirectX::XMVECTOR minPoint = DirectX::XMVectorSet(-40.0f, -40.0f, -20.0f, 0.0f);
	DirectX::XMVECTOR maxPoint = DirectX::XMVectorSet(20, 20, 0.0f, 0.0f);
	DirectX::BoundingBox::CreateFromPoints(
		root->boundingBox,
		minPoint,
		maxPoint
	);
}


template<typename T>
void OcTree<T>::AddElement(const T* elementAddress)
{
	AddToNode(elementAddress, root, 1);
};

// culling
template<typename T>
std::vector<const T*> OcTree<T>::CheckTree(const DirectX::BoundingFrustum& frustum)
{
	std::vector<const T*> toReturn;
	CheckNode(frustum, root, toReturn);
	return toReturn;
};

template<typename T>
std::string OcTree<T>::ToGraphviz()
{
	std::string toReturn = "digraph D{\n";
	size_t counter = 0;
	GenerateGraphvizLinks(toReturn, root, counter);
	return toReturn + '}';
}