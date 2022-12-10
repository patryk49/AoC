#pragma once

#include "Utils.hpp"
#include <initializer_list>
#include <type_traits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"

namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////


template<class T>
struct ForwardList{
	ForwardList() noexcept;
	ForwardList(const ForwardList &list) noexcept;
	ForwardList(ForwardList &&list) noexcept;
	~ForwardList() noexcept;

	ForwardList &operator =(const ForwardList &list) noexcept;
	ForwardList &operator =(ForwardList &&list) noexcept;

	T &operator [](const uint32_t index) noexcept;
	[[nodiscard]] const T &operator [](const uint32_t index) const noexcept;
	[[nodiscard]] uint32_t size() const;
	void resize(const uint32_t size) noexcept;

	void pushFront(const T &newData) noexcept;
	void popFront() noexcept;

private:
	struct Node{
		T data;
		Node *next;
	};
	Node *head;

public:
	struct It : public std::iterator<std::forward_iterator_tag ,T>{
		It() noexcept;
		It(const It &iterator) noexcept;
		It(const ForwardList &list) noexcept;

		It &operator =(const It &iterator) noexcept;
		It &operator =(const ForwardList &list) noexcept;
		
		[[nodiscard]] It operator +(const uint32_t steps) noexcept;
		It &operator +=(const uint32_t steps) noexcept;
		It &operator ++() noexcept;
		T &operator *() noexcept;
		T &operator [](const uint32_t index) noexcept;

		[[nodiscard]] uint32_t operator -(const It iter) const noexcept;

		[[nodiscard]] bool operator ==(const It iterator) const;
		[[nodiscard]] bool operator !=(const It iterator) const;

		void insertBefore(const T &newData) noexcept;
		void removeNext() noexcept;

		Node *ptr;
	};

	[[nodiscard]] constexpr It begin() const noexcept;
	[[nodiscard]] constexpr It end() const noexcept;
};

template<class T>
struct LinkedList{
	LinkedList() noexcept;
	LinkedList(const LinkedList &list) noexcept;
	LinkedList(LinkedList &&list) noexcept;
	~LinkedList() noexcept;

	LinkedList &operator =(const LinkedList &list) noexcept;
	LinkedList &operator =(LinkedList &&list) noexcept;

	T &operator [](const uint32_t index) noexcept;
	[[nodiscard]] const T &operator [](const uint32_t index) const noexcept;
	[[nodiscard]] uint32_t size() const;
	void resize(const uint32_t size) noexcept; 

	void pushFront(const T &newData) noexcept;
	void popFront() noexcept;

private:
	struct Node{
		T data;
		Node *next;
		Node *prev;
	};
	Node *head;

public:
	struct It : public std::iterator<std::random_access_iterator_tag, T>{
		It() noexcept;
		It(const It &iterator) noexcept;
		It(const LinkedList &list) noexcept;

		It &operator =(const It &iterator) noexcept;
		It &operator =(const LinkedList &list) noexcept;
		
		[[nodiscard]] It operator +(const uint32_t steps) const noexcept;
		[[nodiscard]] It operator -(const uint32_t steps) const noexcept;
		It &operator +=(const uint32_t steps) noexcept;
		It &operator -=(const uint32_t steps) noexcept;
		It &operator ++() noexcept;
		It &operator --() noexcept;
		T &operator *() noexcept;

		[[nodiscard]] uint32_t operator -(const It iter) const noexcept;

		[[nodiscard]] bool operator ==(const It iterator) const;
		[[nodiscard]] bool operator !=(const It iterator) const;

		void insertBefore(const T &newData) noexcept;
		void removeNext() noexcept;
		void insertBehind(const T &newData) noexcept;
		void removePrev() noexcept;

		Node *ptr;
	};

	[[nodiscard]] constexpr It begin() const noexcept;
	[[nodiscard]] constexpr It end() const noexcept;
};






#define TPL template<class T>
#define CLS ForwardList<T>

TPL CLS::ForwardList() noexcept : head{nullptr} {}

TPL CLS::ForwardList(const CLS &list) noexcept{
	Node *I = head;
	if (list.head){
		I = (Node *)malloc(sizeof(Node));
		I->data = list.head->data;
		for (Node *J=list.head->next; J!=nullptr; J=J->next){
			I->next = (Node *)malloc(sizeof(Node));
			I = I->next;
			I->data = J->data;
		}
	}
	I = nullptr;
}

TPL CLS::ForwardList(CLS &&list) noexcept : head{list.head}{
	~ForwardList();
	list.head = nullptr;
}

TPL CLS::~ForwardList() noexcept{
	Node *I = head;
	while (I){
		Node *const toDelete = I;
		I = I->next;
		free(toDelete);
	}
}

TPL CLS &CLS::operator =(const CLS &list) noexcept{
	Node *I = head;
	Node *J = list.head;
	Node *lastNode = head;
	for (; I && J; I=I->next, J=J->next){
		I->data = J->data;
		lastNode = I;
	}

	if (I){
		while (I){
			J = I;
			I = I->next;
			free(J);
		}
	} else{
		if (!head & J){
			head = (Node *)malloc(sizeof(Node));
			head->data = J->data;
			lastNode = head;
			J = J->next;
		}
		for (; J; J=J->next){
			I = (Node *)malloc(sizeof(Node));
			I->data = J->data;
			lastNode->next = I;
			lastNode = I;
		}
	}
	lastNode->next = nullptr;
	return *this;
}

TPL CLS &CLS::operator =(CLS &&list) noexcept{
	~ForwardList();
	head = list.head;
	list.head = nullptr;
	return *this;
}

TPL void CLS::pushFront(const T &newData) noexcept{
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->data = newData;
	newNode->next = head;
	head = newNode;
}

TPL void CLS::popFront() noexcept{
	Node *toDelete = head;
	head = head->next;
	free(toDelete);
}

TPL T &CLS::operator [](const uint32_t index) noexcept{
	Node *I = head;
	for (uint32_t i=0; i!=index; ++i)
		I = I->next;
	return I->data;
}

TPL T const &CLS::operator [](const uint32_t index) const noexcept{
	Node *I = head;
	for (uint32_t i=0; i!=index; ++i)
		I = I->next;
	return I->data;
}

TPL uint32_t CLS::size() const{
	uint32_t i = 0;
	for (Node *I=head; I; I=I->next, ++i);
	return i;
}

TPL void CLS::resize(const uint32_t size) noexcept{
	uint32_t lenght = 0;
	Node *I = head;
	Node *lastNode = head;
	for (; lenght<size && I; ++lenght, I=I->next)
		lastNode = I;

	if (I){
		lastNode->next = nullptr;
		while (I){
			lastNode = I;
			I = I->next;
			free(lastNode);
		}
	} else{
		if (!head && size){
			I = (Node *)malloc(sizeof(Node));
			lastNode = I;
			++lenght;
		}
		for (;lenght<size; ++lenght){
			I = (Node *)malloc(sizeof(Node));
			lastNode->next = I;
			lastNode = I;
		}
		lastNode->next = nullptr;
	}
}

TPL constexpr typename CLS::It CLS::begin() const noexcept{
	return CLS::It(*this);
};

TPL constexpr typename CLS::It CLS::end() const noexcept{
	CLS::It result;
	result.ptr = nullptr;
	return result;
};


TPL CLS::It::It() noexcept{}

TPL CLS::It::It(const It &iterator) noexcept{
	ptr = iterator.ptr;
}

TPL CLS::It::It(const CLS &list) noexcept{
	ptr = list.head;
}

TPL typename CLS::It &CLS::It::operator =(const It &iterator) noexcept{
	ptr = iterator.ptr;
	return *this;
}

TPL typename CLS::It &CLS::It::operator =(const CLS &list) noexcept{
	ptr = list.head;
	return *this;
}

TPL typename CLS::It CLS::It::operator +(const uint32_t steps) noexcept{
	CLS::It result;
	result.ptr = this->ptr;
	for (uint32_t i=0; i!=steps; ++i)
		result.ptr = result.ptr->next;
	return result;
}

TPL typename CLS::It &CLS::It::operator +=(const uint32_t steps) noexcept{
	for (uint32_t i=0; i!=steps; ++i)
		this->ptr = this->ptr->next;
	return *this;
}

TPL typename CLS::It &CLS::It::operator ++() noexcept{
	ptr = ptr->next;
	return *this;
}

TPL T &CLS::It::operator *() noexcept{
	return ptr->data;
}

TPL T &CLS::It::operator [](const uint32_t index) noexcept{
	Node *I = ptr;
	for (uint32_t i=0; i!=index; ++i)
		I = I->next;
	return I->data;
}

TPL uint32_t CLS::It::operator -(const It iter) const noexcept{
	uint32_t result = 0;
	for (Node *I=iter.ptr; I!=this->ptr; I=I->next, ++result);
	return result;
}

TPL bool CLS::It::operator ==(const It iterator) const{
	return ptr == iterator.ptr;
}

TPL bool CLS::It::operator !=(const It iterator) const{
	return ptr != iterator.ptr;
}

TPL void CLS::It::insertBefore(const T &newData) noexcept{
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->data = newData;
	newNode->next = ptr->next;
	ptr->next = newNode;
}

TPL void CLS::It::removeNext() noexcept{
	Node *nodePtr = ptr->next;
	ptr->next = nodePtr->next;
	free(nodePtr);
}

#undef TPL
#undef CLS



#define TPL template<class T>
#define CLS LinkedList<T>

TPL CLS::LinkedList() noexcept{
	head = nullptr;
}

TPL CLS::LinkedList(const CLS &list) noexcept{
	Node *I = head;
	if (list.head){
		I = (Node *)malloc(sizeof(Node));
		I->prev = nullptr;
		I->data = list.head->data;
		for (Node *J=list.head->next; J; J=J->next){
			I->next = (Node *)malloc(sizeof(Node));
			I->next->prev = I;
			I = I->next;
			I->data = J->data;
		}
	}
	I = nullptr;
}

TPL CLS::LinkedList(CLS &&list) noexcept :head{list.head}{
	~LinkedList();
	list.head = nullptr;
}

TPL CLS::~LinkedList() noexcept{
	Node *I = head;
	while (I){
		Node *const toDelete = I;
		I = I->next;
		free(toDelete);
	}
}

TPL CLS &CLS::operator =(const CLS &list) noexcept{
	Node *I = head;
	Node *J = list.head;
	Node *lastNode = head;
	for (; I && J; I=I->next, J=J->next){
		I->data = J->data;
		lastNode = I;
	}

	if (I){
		while (I){
			J = I;
			I = I->next;
			free(J);
		}
	} else{
		if (!head && J){
			head = (Node *)malloc(sizeof(Node));
			head->data = J->data;
			head->prev = nullptr;
			lastNode = head;
			J = J->next;
		}
		for (; J; J=J->next){
			I = (Node *)malloc(sizeof(Node));
			I->data = J->data;
			I->prev = lastNode;
			lastNode->next = I;
			lastNode = I;
		}
	}
	lastNode->next = nullptr;
	return *this;
}

TPL CLS &CLS::operator =(CLS &&list) noexcept{
	~LinkedList();
	head = list.head;
	list.head = nullptr;
	return *this;
}

TPL void CLS::pushFront(const T &newData) noexcept{
	Node *const newNode = (Node *)malloc(sizeof(Node));
	newNode->data = newData;
	newNode->next = head;
	newNode->prev = nullptr;
	if (head)
		head->prev = newNode;
	head = newNode;
}

TPL void CLS::popFront() noexcept{
	Node *const toDelete = head;
	head = head->next;
	head->prev = nullptr;
	free(toDelete);
}

TPL T &CLS::operator [](const uint32_t index) noexcept{
	Node *I = head;
	for (uint32_t i=0; i!=index; ++i)
		I = I->next;
	return I->data;
}

TPL T const &CLS::operator [](const uint32_t index) const noexcept{
	Node *I = head;
	for (uint32_t i=0; i!=index; ++i)
		I = I->next;
	return I->data;
}

TPL uint32_t CLS::size() const{
	uint32_t i = 0;
	for (Node *I=head; I; I=I->next, ++i);
	return i;
}

TPL void CLS::resize(const uint32_t size) noexcept{
	uint32_t lenght = 0;
	Node *I = head;
	Node *lastNode = head;
	for (; lenght<size && I; ++lenght, I=I->next)
		lastNode = I;

	if (I){
		lastNode->next = nullptr;
		while (I){
			lastNode = I;
			I = I->next;
			free(lastNode);
		}
	} else{
		if (!head && size){
			head = (Node *)malloc(sizeof(Node));
			head->prev = nullptr;
			lastNode = head;
			++lenght;
		}
		for (;lenght<size; ++lenght){
			I = (Node *)malloc(sizeof(Node));
			I->prev = lastNode;
			lastNode->next = I;
			lastNode = I;
		}
		lastNode->next = nullptr;
	}
}

TPL constexpr typename CLS::It CLS::begin() const noexcept{
	return CLS::It(*this);
};

TPL constexpr typename CLS::It CLS::end() const noexcept{
	CLS::It result;
	result.ptr = nullptr;
	return result;
};


TPL CLS::It::It() noexcept{}

TPL CLS::It::It(const It &iterator) noexcept{
	ptr = iterator.ptr;
}

TPL CLS::It::It(const CLS &list) noexcept{
	ptr = list.head;
}

TPL typename CLS::It &CLS::It::operator =(const It &iterator) noexcept{
	ptr = iterator.ptr;
	return *this;
}

TPL typename CLS::It &CLS::It::operator =(const CLS &list) noexcept{
	ptr = list.head;
	return *this;
}

TPL typename CLS::It CLS::It::operator +(const uint32_t steps) const noexcept{
	CLS::It result;
	result.ptr = this->ptr;
	for (uint32_t i=0; i!=steps; ++i)
		result.ptr = result.ptr->next;
	return result;
}

TPL typename CLS::It CLS::It::operator -(const uint32_t steps) const noexcept{
	CLS::It result;
	result.ptr = this->ptr;
	for (uint32_t i=0; i!=steps; ++i)
		result.ptr = result.ptr->prev;
	return result;
}

TPL typename CLS::It &CLS::It::operator +=(const uint32_t steps) noexcept{
	for (uint32_t i=0; i!=steps; ++i)
		this->ptr = this->ptr->next;
	return *this;
}

TPL typename CLS::It &CLS::It::operator -=(const uint32_t steps) noexcept{
	for (uint32_t i=0; i<steps; ++i)
		this->ptr = this->ptr->prev;
	return *this;
}

TPL typename CLS::It &CLS::It::operator ++() noexcept{
	ptr = ptr->next;
	return *this;
}

TPL typename CLS::It &CLS::It::operator --() noexcept{
	ptr = ptr->prev;
	return *this;
}

TPL T &CLS::It::operator *() noexcept{
	return ptr->data;
}

TPL uint32_t CLS::It::operator -(const It iter) const noexcept{
	uint32_t result = 0;
	for (Node *I=iter.ptr; I!=this->ptr; I=I->next, ++result);
	return result;
}

TPL bool CLS::It::operator ==(const It iterator) const{
	return ptr == iterator.ptr;
}

TPL bool CLS::It::operator !=(const It iterator) const{
	return ptr != iterator.ptr;
}

TPL void CLS::It::insertBefore(const T &newData) noexcept{
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->data = newData;
	newNode->next = ptr->next;
	newNode->prev = ptr;
	ptr->next = newNode;
}

TPL void CLS::It::removeNext() noexcept{
	Node *nodePtr = ptr->next;
	ptr->next = nodePtr->next;
	nodePtr->next->prev = ptr;
	free(nodePtr);
}

TPL void CLS::It::insertBehind(const T &newData) noexcept{
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->data = newData;
	newNode->next = ptr;
	newNode->prev = ptr->prev;
	ptr->prev = newNode;
}

TPL void CLS::It::removePrev() noexcept{
	Node *nodePtr = ptr->prev;
	ptr->prev = nodePtr->prev;
	nodePtr->prev->next = ptr;
	free(nodePtr);
}

#undef TPL
#undef CLS


} // END OF NAMESPACE SP ///////////////////////////////////////////////////////////////////////////////