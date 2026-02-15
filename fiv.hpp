#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <string>

#include <cstddef>
#include <limits>

namespace fiv
{

	// A unique identifier for an element in the vector. It is designed to be opaque and only used through the Vector class.
	// Note! IDs are recycled after an element is removed.
	struct ID
	{
	private:
		std::size_t value;

		inline operator std::size_t() const { return value; }

	public:
		ID() : value(std::numeric_limits<std::size_t>::max()) {}
		ID(const ID &other) = default;
		explicit ID(std::size_t value) : value(value) {}

		ID &operator=(const ID &other) = default;

		inline bool operator==(const ID &other) const
		{
			return value == other.value;
		}

		inline bool operator!=(const ID &other) const
		{
			return value != other.value;
		}

		inline bool isInvalid() const
		{
			return value == std::numeric_limits<std::size_t>::max();
		}

		inline void invalidate()
		{
			value = std::numeric_limits<std::size_t>::max();
		}

		template <typename T>
		friend class Vector;
	};

	template <typename T>
	class Vector
	{
	private:
		bool m_KeepOrder;

		// m_Data stores the actual elements of the vector.
		std::vector<T> m_Data;

		// m_IDs maps an ID index to the corresponding index in m_Data. The position of an ID in m_IDs is its ID value.
		std::vector<size_t> m_IDs;

		// m_RevIDs maps an index in m_Data to the corresponding ID index. The position of an element in m_Data is its data index.
		std::vector<ID> m_RevIDs;

		// m_FreeIDSlots stores the indices of IDs in m_IDs that have been removed and can be reused for new elements.
		std::vector<ID> m_FreeIDSlots;

	public:
		Vector() : m_KeepOrder(true) {}
		Vector(bool keepOrder) : m_KeepOrder(keepOrder) {}
		Vector(const Vector &other) = default;

	public:
		void reserve(size_t numElements)
		{
			m_Data.reserve(numElements);
			m_IDs.reserve(numElements - m_FreeIDSlots.size());
			m_RevIDs.reserve(numElements);
		}

	private:
		ID addID(ID location)
		{
			ID idIndex{m_IDs.size()};
			if (!m_FreeIDSlots.empty())
			{
				idIndex = m_FreeIDSlots.back();
				m_FreeIDSlots.pop_back();
				m_IDs[idIndex.value] = location.value;
			}
			else
			{
				m_IDs.push_back(location.value);
			}
			return idIndex;
		}

		// Swaps the locations of two elements in the data vector, and updates the corresponding IDs and reverse IDs.
		// Does not swap the data that the IDs point to, only the locations of the IDs in the data vector.
		void swapDataLocation(ID el1, ID el2)
		{
			size_t idx1 = m_IDs[el1.value];
			size_t idx2 = m_IDs[el2.value];
			std::swap(m_IDs[el1.value], m_IDs[el2.value]);
			std::swap(m_Data[idx1], m_Data[idx2]);
			std::swap(m_RevIDs[idx1], m_RevIDs[idx2]);
		}

	public:
		void remove(ID id)
		{
			size_t dataIndex = m_IDs[id.value];
			ID lastDataID = m_RevIDs.back();

			if (m_KeepOrder)
			{
				for (size_t i = indexOf(id); i < m_Data.size() - 1; i++)
				{
					swapDataLocation(idAt(i), idAt(i + 1));
				}
			}
			else
			{
				swapDataLocation(id, lastDataID);
			}

			m_FreeIDSlots.push_back(id);
			m_Data.pop_back();
			m_RevIDs.pop_back();
			if (m_Data.empty())
			{
				clear();
			}
		}

		void clear()
		{
			m_Data.clear();
			m_IDs.clear();
			m_RevIDs.clear();
			m_FreeIDSlots.clear();
		}

		template <typename... Args>
		ID emplace(Args &&...args)
		{
			m_Data.emplace_back(std::forward<Args>(args)...);
			ID id{m_Data.size() - 1};
			ID idIndex = addID(id);
			m_RevIDs.push_back(idIndex);
			return idIndex;
		}

		ID push(T element)
		{
			m_Data.push_back(element);
			ID id = ID(m_Data.size() - 1);
			ID idIndex = addID(id);
			m_RevIDs.push_back(idIndex);
			return idIndex;
		}

		T &get(ID id)
		{
			if (validID(id))
			{
				return m_Data[m_IDs[id.value]];
			}
			else
			{
				std::string faliureReason = "";
				if (id.value < 0 || id.value >= m_IDs.size())
				{
					faliureReason = "ID out of bounds.";
				}
				else
				{
					faliureReason = "The object with this id has been deleted.";
				}
				throw std::invalid_argument("ID: " + std::to_string(id.value) + " is not a valid ID. " + faliureReason);
			}
		}

		const T &get(ID id) const
		{
			if (validID(id))
			{
				return m_Data[m_IDs[id.value]];
			}
			else
			{
				std::string faliureReason;
				if (id.value < 0 || id.value >= m_IDs.size())
				{
					faliureReason = "ID out of bounds.";
				}
				else
				{
					faliureReason = "The object with this id has been deleted.";
				}
				throw std::invalid_argument("ID: " + std::to_string(id.value) + " is not a valid ID. " + faliureReason);
			};
		}

		inline ID idAt(size_t index) const
		{
			return m_RevIDs[index];
		}

		bool validID(ID id)
		{
			return id.value >= 0 && id.value < m_IDs.size() && std::find(m_FreeIDSlots.begin(), m_FreeIDSlots.end(), id) == m_FreeIDSlots.end();
		}

		inline const size_t size() const
		{
			return m_Data.size();
		}

		inline T &operator[](ID id)
		{
			return m_Data[m_IDs[id.value]];
		}

		inline const T &operator[](ID id) const
		{
			return m_Data[m_IDs[id.value]];
		}

		inline T &dataAt(size_t index)
		{
			return m_Data[index];
		}

		inline const T &dataAt(size_t index) const
		{
			return m_Data[index];
		}

		inline size_t indexOf(ID id) const
		{
			return m_IDs[id.value];
		}

		inline const std::vector<T> &data() const
		{
			return m_Data;
		}

		inline std::vector<T> &data()
		{
			return m_Data;
		}

		inline typename std::vector<T>::iterator begin()
		{
			return m_Data.begin();
		}

		inline typename std::vector<T>::iterator end()
		{
			return m_Data.end();
		}

		inline typename std::vector<T>::const_iterator begin() const
		{
			return m_Data.begin();
		}

		inline typename std::vector<T>::const_iterator end() const
		{
			return m_Data.end();
		}
	};

	// An ID that will not pass any validity tests.
	const ID invalidID = ID(std::numeric_limits<std::size_t>::max());

}
