#include "native/net_class_editor.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "native/log.h"

#include "dt_common.h"

namespace gm_laboratory {
	const char* NetPropTypeName(int t) {
		switch (t) {
		case DPT_Int:       return "int";
		case DPT_Float:     return "float";
		case DPT_Vector:    return "vector";
		case DPT_VectorXY:  return "vectorxy";
		case DPT_String:    return "string";
		case DPT_Array:     return "array";
		case DPT_DataTable: return "datatable";
#ifdef SUPPORTS_INT64
		case DPT_Int64:     return "int64";
#endif
		default:            return "?";
		}
	}

	static constexpr int kMaxDepth = 16;

	template <class Table, class Prop>
	static Prop* FindPropRecursive(Table* table, const char* path, int depth, Table** outOwner, int* outIndex) {
		if (!table || !path || depth > kMaxDepth)
			return nullptr;

		const char* slash = std::strchr(path, '/');

		if (slash) {
			std::size_t segLen = static_cast<std::size_t>(slash - path);
			for (int i = 0; i < table->GetNumProps(); ++i) {
				Prop* prop = table->GetProp(i);
				if (prop->GetType() != DPT_DataTable)
					continue;

				Table* child = prop->GetDataTable();
				const char* propName = prop->GetName();
				const char* childName = child ? child->GetName() : nullptr;
				bool match = (propName && std::strlen(propName) == segLen && std::strncmp(propName, path, segLen) == 0)
					|| (childName && std::strlen(childName) == segLen && std::strncmp(childName, path, segLen) == 0);
				if (match)
					return FindPropRecursive<Table, Prop>(child, slash + 1, depth + 1, outOwner, outIndex);
			}
			return nullptr;
		}

		for (int i = 0; i < table->GetNumProps(); ++i) {
			Prop* prop = table->GetProp(i);
			const char* name = prop->GetName();
			if (name && std::strcmp(name, path) == 0) {
				*outOwner = table;
				*outIndex = i;
				return prop;
			}
		}

		for (int i = 0; i < table->GetNumProps(); ++i) {
			Prop* prop = table->GetProp(i);
			if (prop->GetType() != DPT_DataTable)
				continue;
			if (Prop* found = FindPropRecursive<Table, Prop>(prop->GetDataTable(), path, depth + 1, outOwner, outIndex))
				return found;
		}

		return nullptr;
	}

	template <class Table, class Prop>
	static void DumpTable(Table* table, int depth) {
		if (!table || depth > kMaxDepth)
			return;

		for (int i = 0; i < table->GetNumProps(); ++i) {
			Prop* prop = table->GetProp(i);
			const char* name = prop->GetName();
			int type = prop->GetType();
			Log("netclass", "%*s%s : %s (offset %d, flags 0x%X)\n",
				depth * 2, "", name ? name : "?", NetPropTypeName(type), prop->GetOffset(), prop->GetFlags());
			if (type == DPT_DataTable)
				DumpTable<Table, Prop>(prop->GetDataTable(), depth + 1);
		}
	}

	template <class Table, class Prop>
	static Prop* AppendProp(Table* table, const Prop& prop) {
		const int n = table->m_nProps;
		const std::size_t sz = sizeof(Prop);
		auto* arr = static_cast<Prop*>(std::malloc(sz * (static_cast<std::size_t>(n) + 1)));
		if (!arr)
			return nullptr;
		std::memcpy(arr, table->m_pProps, sz * static_cast<std::size_t>(n));
		std::memcpy(reinterpret_cast<char*>(arr) + sz * static_cast<std::size_t>(n), &prop, sz);
		table->m_pProps = arr;
		table->m_nProps = n + 1;
		return &arr[n];
	}

	template <class Table>
	static bool RemovePropAt(Table* table, int index) {
		if (!table)
			return false;
		const int n = table->m_nProps;
		if (index < 0 || index >= n)
			return false;
		const std::size_t sz = sizeof(typename Table::PropType);
		auto* bytes = reinterpret_cast<char*>(table->m_pProps);
		std::memmove(bytes + sz * static_cast<std::size_t>(index),
			bytes + sz * static_cast<std::size_t>(index + 1),
			sz * static_cast<std::size_t>(n - 1 - index));
		table->m_nProps = n - 1;
		return true;
	}

	void RecvPropEditor::Remove() {
		if (m_prop && m_owner && m_index >= 0 && RemovePropAt(m_owner, m_index)) {
			m_prop = nullptr;
			m_index = -1;
		}
	}

	void SendPropEditor::Remove() {
		if (m_prop && m_owner && m_index >= 0 && RemovePropAt(m_owner, m_index)) {
			m_prop = nullptr;
			m_index = -1;
		}
	}

	void ClientClassEditor::ForEachProp(const ForEachPropFn& fn) const {
		RecvTable* t = Table();
		if (!t)
			return;
		for (int i = 0; i < t->GetNumProps(); ++i) {
			RecvProp* p = t->GetProp(i);
			fn(p->GetName() ? p->GetName() : "?", p->GetType(), p->GetOffset(), p->GetFlags());
		}
	}

	void ClientClassEditor::Dump() const {
		RecvTable* t = Table();
		Log("netclass", "[client] %s -> %s\n", Name() ? Name() : "?", (t && t->GetName()) ? t->GetName() : "<no table>");
		DumpTable<RecvTable, RecvProp>(t, 1);
	}

	RecvPropEditor ClientClassEditor::GetProp(const char* propPath) const {
		RecvTable* t = Table();
		if (!t)
			return {};

		RecvTable* owner = nullptr;
		int index = -1;
		RecvProp* prop = FindPropRecursive<RecvTable, RecvProp>(t, propPath, 0, &owner, &index);
		if (prop)
			return RecvPropEditor(owner, prop, index);
		return {};
	}

	RecvPropEditor ClientClassEditor::AddProp(const RecvProp& prop) const {
		return AddProp(Table(), prop);
	}

	RecvPropEditor ClientClassEditor::AddProp(RecvTable* table, const RecvProp& prop) const {
		if (!table)
			return {};
		if (RecvProp* added = AppendProp<RecvTable, RecvProp>(table, prop))
			return RecvPropEditor(table, added, table->GetNumProps() - 1);
		return {};
	}

	bool ClientClassEditor::RemoveProp(const char* propPath) const {
		RecvPropEditor prop = GetProp(propPath);
		if (!prop.Valid())
			return false;
		prop.Remove();
		return true;
	}

	void ServerClassEditor::ForEachProp(const ForEachPropFn& fn) const {
		SendTable* t = Table();
		if (!t)
			return;
		for (int i = 0; i < t->GetNumProps(); ++i) {
			SendProp* p = t->GetProp(i);
			fn(p->GetName() ? p->GetName() : "?", p->GetType(), p->GetOffset(), p->GetFlags());
		}
	}

	void ServerClassEditor::Dump() const {
		SendTable* t = Table();
		Log("netclass", "[server] %s -> %s\n", Name() ? Name() : "?", (t && t->GetName()) ? t->GetName() : "<no table>");
		DumpTable<SendTable, SendProp>(t, 1);
	}

	SendPropEditor ServerClassEditor::GetProp(const char* propPath) const {
		SendTable* t = Table();
		if (!t)
			return {};

		SendTable* owner = nullptr;
		int index = -1;
		SendProp* prop = FindPropRecursive<SendTable, SendProp>(t, propPath, 0, &owner, &index);
		if (prop)
			return SendPropEditor(owner, prop, index);
		return {};
	}

	SendPropEditor ServerClassEditor::AddProp(const SendProp& prop) const {
		return AddProp(Table(), prop);
	}

	SendPropEditor ServerClassEditor::AddProp(SendTable* table, const SendProp& prop) const {
		if (!table)
			return {};
		if (SendProp* added = AppendProp<SendTable, SendProp>(table, prop))
			return SendPropEditor(table, added, table->GetNumProps() - 1);
		return {};
	}

	bool ServerClassEditor::RemoveProp(const char* propPath) const {
		SendPropEditor prop = GetProp(propPath);
		if (!prop.Valid())
			return false;
		prop.Remove();
		return true;
	}

	void ClientClassesEditor::ForEachClass(const ForEachClassFn& fn) const {
		for (ClientClass* c = ListStart; c; c = c->m_pNext)
			fn(c->m_pNetworkName ? c->m_pNetworkName : "?");
	}

	void ClientClassesEditor::Dump() const {
		for (ClientClass* c = ListStart; c; c = c->m_pNext)
			ClientClassEditor(c).Dump();
	}

	ClientClassEditor ClientClassesEditor::GetClass(const char* networkName) const {
		for (ClientClass* c = ListStart; c; c = c->m_pNext)
			if (c->m_pNetworkName && std::strcmp(c->m_pNetworkName, networkName) == 0)
				return ClientClassEditor(c);
		return ClientClassEditor();
	}

	void ServerClassesEditor::ForEachClass(const ForEachClassFn& fn) const {
		for (ServerClass* c = ListStart; c; c = c->m_pNext)
			fn(c->m_pNetworkName ? c->m_pNetworkName : "?");
	}

	void ServerClassesEditor::Dump() const {
		for (ServerClass* c = ListStart; c; c = c->m_pNext)
			ServerClassEditor(c).Dump();
	}

	ServerClassEditor ServerClassesEditor::GetClass(const char* networkName) const {
		for (ServerClass* c = ListStart; c; c = c->m_pNext)
			if (c->m_pNetworkName && std::strcmp(c->m_pNetworkName, networkName) == 0)
				return ServerClassEditor(c);
		return ServerClassEditor();
	}
}
