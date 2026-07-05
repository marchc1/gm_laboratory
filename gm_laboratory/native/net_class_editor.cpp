#include "native/net_class_editor.h"

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


	RecvVarProxyFn RecvPropHandle::OriginalProxy() const { return Prop ? Prop->GetProxyFn() : nullptr; }
	void RecvPropHandle::RerouteProxy(RecvVarProxyFn fn) { if (Prop) Prop->SetProxyFn(fn); }
	void RecvPropHandle::SetFlags(int flags) { if (Prop) Prop->m_Flags = flags; }
	void RecvPropHandle::AddFlags(int flags) { if (Prop) Prop->m_Flags |= flags; }
	void RecvPropHandle::StripFlags(int flags) { if (Prop) Prop->m_Flags &= ~flags; }
	void RecvPropHandle::SetOffset(int offset) { if (Prop) Prop->SetOffset(offset); }

	SendVarProxyFn SendPropHandle::OriginalProxy() const { return Prop ? Prop->GetProxyFn() : nullptr; }
	void SendPropHandle::RerouteProxy(SendVarProxyFn fn) { if (Prop) Prop->SetProxyFn(fn); }
	void SendPropHandle::SetFlags(int flags) { if (Prop) Prop->SetFlags(flags); }
	void SendPropHandle::AddFlags(int flags) { if (Prop) Prop->SetFlags(Prop->GetFlags() | flags); }
	void SendPropHandle::StripFlags(int flags) { if (Prop) Prop->SetFlags(Prop->GetFlags() & ~flags); }
	void SendPropHandle::SetOffset(int offset) { if (Prop) Prop->SetOffset(offset); }


	ClientClass* ClientClassEditor::GetClass(const char* networkName) const {
		for (ClientClass* c = ListStart; c; c = c->m_pNext)
			if (c->m_pNetworkName && std::strcmp(c->m_pNetworkName, networkName) == 0)
				return c;

		return nullptr;
	}

	void ClientClassEditor::ForEachClass(const ForEachClassFn& fn) const {
		for (ClientClass* c = ListStart; c; c = c->m_pNext)
			fn(c->m_pNetworkName ? c->m_pNetworkName : "?");
	}

	void ClientClassEditor::ForEachProp(const char* className, const ForEachPropFn& fn) const {
		ClientClass* c = GetClass(className);
		if (!c || !c->m_pRecvTable)
			return;
		RecvTable* t = c->m_pRecvTable;
		for (int i = 0; i < t->GetNumProps(); ++i) {
			RecvProp* p = t->GetProp(i);
			fn(p->GetName() ? p->GetName() : "?", p->GetType(), p->GetOffset(), p->GetFlags());
		}
	}

	void ClientClassEditor::Dump() const {
		for (ClientClass* c = ListStart; c; c = c->m_pNext) {
			RecvTable* t = c->m_pRecvTable;
			Log("netclass", "[client] %s -> %s\n", c->m_pNetworkName ? c->m_pNetworkName : "?", (t && t->GetName()) ? t->GetName() : "<no table>");
			DumpTable<RecvTable, RecvProp>(t, 1);
		}
	}

	RecvPropHandle ClientClassEditor::GetProp(const char* className, const char* propPath) const {
		RecvPropHandle handle;
		ClientClass* c = GetClass(className);
		if (!c || !c->m_pRecvTable)
			return handle;

		RecvTable* owner = nullptr;
		int index = -1;
		RecvProp* prop = FindPropRecursive<RecvTable, RecvProp>(c->m_pRecvTable, propPath, 0, &owner, &index);
		if (prop) {
			handle.Table = owner;
			handle.Prop = prop;
			handle.Index = index;
			handle.Valid = true;
		}

		return handle;
	}

	ServerClass* ServerClassEditor::GetClass(const char* networkName) const {
		for (ServerClass* c = ListStart; c; c = c->m_pNext)
			if (c->m_pNetworkName && std::strcmp(c->m_pNetworkName, networkName) == 0)
				return c;

		return nullptr;
	}

	void ServerClassEditor::ForEachClass(const ForEachClassFn& fn) const {
		for (ServerClass* c = ListStart; c; c = c->m_pNext)
			fn(c->m_pNetworkName ? c->m_pNetworkName : "?");
	}

	void ServerClassEditor::ForEachProp(const char* className, const ForEachPropFn& fn) const {
		ServerClass* c = GetClass(className);
		if (!c || !c->m_pTable)
			return;
		SendTable* t = c->m_pTable;
		for (int i = 0; i < t->GetNumProps(); ++i) {
			SendProp* p = t->GetProp(i);
			fn(p->GetName() ? p->GetName() : "?", p->GetType(), p->GetOffset(), p->GetFlags());
		}
	}

	void ServerClassEditor::Dump() const {
		for (ServerClass* c = ListStart; c; c = c->m_pNext) {
			SendTable* t = c->m_pTable;
			Log("netclass", "[server] %s -> %s\n", c->m_pNetworkName ? c->m_pNetworkName : "?", (t && t->GetName()) ? t->GetName() : "<no table>");
			DumpTable<SendTable, SendProp>(t, 1);
		}
	}

	SendPropHandle ServerClassEditor::GetProp(const char* className, const char* propPath) const {
		SendPropHandle handle;
		ServerClass* c = GetClass(className);
		if (!c || !c->m_pTable)
			return handle;

		SendTable* owner = nullptr;
		int index = -1;
		SendProp* prop = FindPropRecursive<SendTable, SendProp>(c->m_pTable, propPath, 0, &owner, &index);
		if (prop) {
			handle.Table = owner;
			handle.Prop = prop;
			handle.Index = index;
			handle.Valid = true;
		}

		return handle;
	}

}
