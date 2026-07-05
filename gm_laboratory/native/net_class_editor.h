#pragma once

#include <functional>

#include "client_class.h"
#include "server_class.h"
#include "dt_recv.h"
#include "dt_send.h"

namespace gm_laboratory {

	const char* NetPropTypeName(int sendPropType);

	struct RecvPropHandle {
		RecvTable* Table = nullptr;
		RecvProp*  Prop  = nullptr;
		int        Index = -1;
		bool       Valid = false;

		RecvVarProxyFn OriginalProxy() const;
		void RerouteProxy(RecvVarProxyFn fn);

		void SetFlags(int flags);
		void AddFlags(int flags);
		void StripFlags(int flags);
		void SetOffset(int offset);
	};

	struct SendPropHandle {
		SendTable* Table = nullptr;
		SendProp*  Prop  = nullptr;
		int        Index = -1;
		bool       Valid = false;

		SendVarProxyFn OriginalProxy() const;
		void RerouteProxy(SendVarProxyFn fn);

		void SetFlags(int flags);
		void AddFlags(int flags);
		void StripFlags(int flags);
		void SetOffset(int offset);
	};

	using ForEachClassFn = std::function<void(const char* className)>;
	using ForEachPropFn  = std::function<void(const char* propName, int type, int offset, int flags)>;

	struct INetClassEditor {
		virtual ~INetClassEditor() = default;

		virtual const char* Side() const = 0;
		virtual void ForEachClass(const ForEachClassFn& fn) const = 0;
		virtual void ForEachProp(const char* className, const ForEachPropFn& fn) const = 0;
		virtual void Dump() const = 0;
	};

	struct ClientClassEditor : INetClassEditor {
		ClientClass* ListStart = nullptr;

		const char* Side() const override { return "client"; }
		void ForEachClass(const ForEachClassFn& fn) const override;
		void ForEachProp(const char* className, const ForEachPropFn& fn) const override;
		void Dump() const override;

		ClientClass* GetClass(const char* networkName) const;

		RecvPropHandle GetProp(const char* className, const char* propPath) const;
	};

	struct ServerClassEditor : INetClassEditor {
		ServerClass* ListStart = nullptr;

		const char* Side() const override { return "server"; }
		void ForEachClass(const ForEachClassFn& fn) const override;
		void ForEachProp(const char* className, const ForEachPropFn& fn) const override;
		void Dump() const override;

		ServerClass* GetClass(const char* networkName) const;
		SendPropHandle GetProp(const char* className, const char* propPath) const;
	};
}
