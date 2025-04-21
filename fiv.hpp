#pragma once
#include <vector>
#include <stdexcept>
#include <string>

namespace fiv {

	using ID = int64_t;

	template<typename T>
	class Vector {
	private:

		bool m_KeepOrder;
		std::vector<T> m_Data;
		std::vector<ID> m_Ids;
		std::vector<ID> m_RevIds;
		std::vector<ID> m_FreeIdSlots;

	public:

		Vector() : m_KeepOrder(true) {}
		Vector(bool keepOrder) : m_KeepOrder(keepOrder) {}
		Vector(const Vector& other) = default;

	public:

		void reserve(size_t numElements) {
			m_Data.reserve(numElements);
			m_Ids.reserve(numElements - m_FreeIdSlots.size());
			m_RevIds.reserve(numElements);
		}

	private:
		ID addID(ID location) {
			ID idIndex = m_Ids.size();
			if (!m_FreeIdSlots.empty()) {
				idIndex = m_FreeIdSlots[m_FreeIdSlots.size() - 1];
				m_FreeIdSlots.pop_back();
				m_Ids[(size_t)idIndex] = location;
			}
			else {
				m_Ids.push_back(location);
			}
			return idIndex;
		}
	public:
		void swap(ID el1, ID el2) {
			std::swap(m_Ids[(size_t)el1], m_Ids[(size_t)el2]);
			std::swap(m_Data[(size_t)m_Ids[(size_t)el1]], m_Data[(size_t)m_Ids[(size_t)el2]]);
			std::swap(m_RevIds[(size_t)m_Ids[(size_t)el1]], m_RevIds[(size_t)m_Ids[(size_t)el2]]);
		}

		void remove(ID id) {
			const ID m_DataRemoveIndex = m_Ids[(size_t)id];
			const ID lastDataID = m_RevIds[(size_t)m_RevIds.size() - 1];
			
			if (m_KeepOrder) {
				for (size_t i = indexOf(id); i < m_Data.size()-1; i++) {
					swap(idAt(i), idAt(i + 1));
				}
			}
			else {
				swap(id, lastDataID);
			}

			m_FreeIdSlots.push_back(id);
			m_Data.pop_back();
			m_RevIds.pop_back();
			if (m_Data.size() == 0) {
				clear();
			}
		}

		void clear() {
			m_Data.clear();
			m_Ids.clear();
			m_RevIds.clear();
			m_FreeIdSlots.clear();
		}

		template<typename ...Args>
		ID emplace(Args&& ...args) {
			m_Data.emplace_back(std::forward<Args>(args)...);
			ID id = m_Data.size() - 1;
			ID idIndex = addID(id);
			m_RevIds.push_back(idIndex);
			return idIndex;
		}

		ID push(T element) {
			m_Data.push_back(element);
			ID id = m_Data.size() - 1;
			ID idIndex = addID(id);
			m_RevIds.push_back(idIndex);
			return idIndex;
		}

		T& get(ID id) {
			if (id >= 0 && id < m_Ids.size() && std::find(m_FreeIdSlots.begin(), m_FreeIdSlots.end(), id) == m_FreeIdSlots.end()) {
				return m_Data[(size_t)m_Ids[(size_t)id]];
			}
			else {
				std::string faliureReason = "";
				if (id < 0 || id > m_Ids.size()) {
					faliureReason = "ID out of bounds.";
				}
				else {
					faliureReason = "The object with this id has been deleted.";
				}
				throw std::invalid_argument("ID: " + std::to_string(id) + " is not a valid ID. " + faliureReason);
			}
		}

		const T& get(ID id) const {
			if (id >= 0 && std::find(m_FreeIdSlots.begin(), m_FreeIdSlots.end(), id) == m_FreeIdSlots.end()) {
				return m_Data[(size_t)m_Ids[(size_t)id]];
			}
			else {
				std::string faliureReason;
				if (id < 0 || id > m_Ids.size()) {
					faliureReason = "ID out of bounds.";
				}
				else {
					faliureReason = "The object with this id has been deleted.";
				}
				throw std::invalid_argument("ID: " + std::to_string(id) + " is not a valid ID. " + faliureReason);
			};
		}

		inline ID idAt(size_t index) const {
			return m_RevIds[index];
		}

		bool validID(ID id) {
			return id >= 0 && id < m_Ids.size() && std::find(m_FreeIdSlots.begin(), m_FreeIdSlots.end(), id) == m_FreeIdSlots.end();
		}

		inline const size_t size() const {
			return m_Data.size();
		}

		inline T& operator [](ID id) {
			return m_Data[(size_t)m_Ids[(size_t)id]];
		}

		inline const T& operator [](ID id) const {
			return m_Data[(size_t)m_Ids[(size_t)id]];
		}

		inline T& dataAt(size_t index) {
			return m_Data[index];
		}

		inline const T& dataAt(size_t index) const {
			return m_Data[index];
		}

		inline size_t indexOf(ID id) const {
			return m_Ids[id];
		}

		inline const std::vector<T>& data() const {
			return m_Data;
		}

		inline std::vector<T>& data() {
			return m_Data;
		}

		inline typename std::vector<T>::iterator begin() {
			return m_Data.begin();
		}

		inline typename std::vector<T>::iterator end() {
			return m_Data.end();
		}

		inline typename std::vector<T>::const_iterator begin() const {
			return m_Data.begin();
		}

		inline typename std::vector<T>::const_iterator end() const {
			return m_Data.end();
		}
	};

	// Returns false if ID cannot be valid. Returns true if ID can be valid.
	static bool validID(ID id) {
		return id >= 0;
	}

	// An ID that will not pass any validity tests.
	const ID invalidID = -1;

}
