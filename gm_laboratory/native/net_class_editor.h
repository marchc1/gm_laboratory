#pragma once

#include <functional>

#include "client_class.h"
#include "server_class.h"
#include "dt_recv.h"
#include "dt_send.h"

namespace gm_laboratory {
	const char* NetPropTypeName(int sendPropType);

	using ForEachClassFn = std::function<void(const char* className)>;
	using ForEachPropFn = std::function<void(const char* propName, int type, int offset, int flags)>;


	struct INetPropEditor {
		virtual ~INetPropEditor() = default;

		virtual bool Valid() const = 0;
		virtual const char* Name() const = 0;
		virtual int Type() const = 0;
		virtual int Offset() const = 0;
		virtual int Flags() const = 0;

		virtual void SetFlags(int flags) = 0;
		virtual void AddFlags(int flags) = 0;
		virtual void RemoveFlags(int flags) = 0;
		virtual void SetOffset(int offset) = 0;
		virtual void Remove() = 0;
	};

	struct RecvPropEditor : INetPropEditor {
		RecvPropEditor() = default;
		RecvPropEditor(RecvTable* owner, RecvProp* prop, int index) : m_owner(owner), m_prop(prop), m_index(index) {}

		RecvProp* Prop() const { return m_prop; }
		RecvTable* OwnerTable() const { return m_owner; }
		int        Index() const { return m_index; }

		bool Valid() const override { return m_prop != nullptr; }
		const char* Name() const override { return m_prop ? m_prop->GetName() : nullptr; }
		int Type() const override { return m_prop ? m_prop->GetType() : -1; }
		int Offset() const override { return m_prop ? m_prop->GetOffset() : -1; }
		int Flags() const override { return m_prop ? m_prop->GetFlags() : 0; }

		void SetFlags(int flags) override { if (m_prop) m_prop->m_Flags = flags; }
		void AddFlags(int flags) override { if (m_prop) m_prop->m_Flags |= flags; }
		void RemoveFlags(int flags) override { if (m_prop) m_prop->m_Flags &= ~flags; }
		void SetOffset(int offset) override { if (m_prop) m_prop->SetOffset(offset); }
		void Remove() override;

		RecvVarProxyFn OriginalProxy() const { return m_prop ? m_prop->GetProxyFn() : nullptr; }
		void RerouteProxy(RecvVarProxyFn fn) { if (m_prop) m_prop->SetProxyFn(fn); }

	private:
		RecvTable* m_owner = nullptr;
		RecvProp* m_prop = nullptr;
		int        m_index = -1;
	};

	struct SendPropEditor : INetPropEditor {
		SendPropEditor() = default;
		SendPropEditor(SendTable* owner, SendProp* prop, int index) : m_owner(owner), m_prop(prop), m_index(index) {}

		SendProp* Prop() const { return m_prop; }
		SendTable* OwnerTable() const { return m_owner; }
		int        Index() const { return m_index; }

		bool Valid() const override { return m_prop != nullptr; }
		const char* Name() const override { return m_prop ? m_prop->GetName() : nullptr; }
		int Type() const override { return m_prop ? m_prop->GetType() : -1; }
		int Offset() const override { return m_prop ? m_prop->GetOffset() : -1; }
		int Flags() const override { return m_prop ? m_prop->GetFlags() : 0; }

		void SetFlags(int flags) override { if (m_prop) m_prop->SetFlags(flags); }
		void AddFlags(int flags) override { if (m_prop) m_prop->SetFlags(m_prop->GetFlags() | flags); }
		void RemoveFlags(int flags) override { if (m_prop) m_prop->SetFlags(m_prop->GetFlags() & ~flags); }
		void SetOffset(int offset) override { if (m_prop) m_prop->SetOffset(offset); }
		void Remove() override;

		SendVarProxyFn OriginalProxy() const { return m_prop ? m_prop->GetProxyFn() : nullptr; }
		void RerouteProxy(SendVarProxyFn fn) { if (m_prop) m_prop->SetProxyFn(fn); }

	private:
		SendTable* m_owner = nullptr;
		SendProp* m_prop = nullptr;
		int        m_index = -1;
	};

	struct INetClassEditor {
		virtual ~INetClassEditor() = default;

		virtual bool Valid() const = 0;
		virtual const char* Name() const = 0;
		virtual const char* TableName() const = 0;
		virtual void ForEachProp(const ForEachPropFn& fn) const = 0;
		virtual void Dump() const = 0;
		virtual bool RemoveProp(const char* propPath) const = 0;
	};

	struct ClientClassEditor : INetClassEditor {
		ClientClassEditor() = default;
		explicit ClientClassEditor(ClientClass* cls) : m_class(cls) {}

		ClientClass* Class() const { return m_class; }
		RecvTable* Table() const { return m_class ? m_class->m_pRecvTable : nullptr; }

		bool Valid() const override { return m_class != nullptr; }
		const char* Name() const override { return m_class ? m_class->m_pNetworkName : nullptr; }
		const char* TableName() const override { RecvTable* t = Table(); return t ? t->GetName() : nullptr; }
		void ForEachProp(const ForEachPropFn& fn) const override;
		void Dump() const override;
		bool RemoveProp(const char* propPath) const override;

		RecvPropEditor GetProp(const char* propPath) const;

		RecvPropEditor AddProp(const RecvProp& prop) const;
		RecvPropEditor AddProp(RecvTable* table, const RecvProp& prop) const;

	private:
		ClientClass* m_class = nullptr;
	};

	struct ServerClassEditor : INetClassEditor {
		ServerClassEditor() = default;
		explicit ServerClassEditor(ServerClass* cls) : m_class(cls) {}

		ServerClass* Class() const { return m_class; }
		SendTable* Table() const { return m_class ? m_class->m_pTable : nullptr; }

		bool Valid() const override { return m_class != nullptr; }
		const char* Name() const override { return m_class ? m_class->m_pNetworkName : nullptr; }
		const char* TableName() const override { SendTable* t = Table(); return t ? t->GetName() : nullptr; }
		void ForEachProp(const ForEachPropFn& fn) const override;
		void Dump() const override;
		bool RemoveProp(const char* propPath) const override;

		SendPropEditor GetProp(const char* propPath) const;

		SendPropEditor AddProp(const SendProp& prop) const;
		SendPropEditor AddProp(SendTable* table, const SendProp& prop) const;

	private:
		ServerClass* m_class = nullptr;
	};

	struct INetClassesEditor {
		virtual ~INetClassesEditor() = default;

		virtual const char* Side() const = 0;
		virtual void ForEachClass(const ForEachClassFn& fn) const = 0;
		virtual void Dump() const = 0;
	};

	struct ClientClassesEditor : INetClassesEditor {
		ClientClassesEditor() = default;
		explicit ClientClassesEditor(ClientClass* head) : ListStart(head) {}

		ClientClass* ListStart = nullptr;

		const char* Side() const override { return "client"; }
		void ForEachClass(const ForEachClassFn& fn) const override;
		void Dump() const override;

		ClientClassEditor GetClass(const char* networkName) const;
	};

	struct ServerClassesEditor : INetClassesEditor {
		ServerClassesEditor() = default;
		explicit ServerClassesEditor(ServerClass* head) : ListStart(head) {}

		ServerClass* ListStart = nullptr;

		const char* Side() const override { return "server"; }
		void ForEachClass(const ForEachClassFn& fn) const override;
		void Dump() const override;

		ServerClassEditor GetClass(const char* networkName) const;
	};
}
