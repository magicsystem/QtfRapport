#ifndef _QtfRapport_QtfRapport_h
#define _QtfRapport_QtfRapport_h

#include <CtrlLib/CtrlLib.h>
#include <Report/Report.h>

using namespace Upp;

class QtfRapport;

struct QRArg {
public:
	typedef VectorMap<String, Value> vars_t;
	
	
	
public:
	QRArg(
		QtfRapport& that_,
		const String& id_
		)
	: that(that_),id(id_) {}
	
	
	QtfRapport& operator()() { return that;}
	QRArg& operator()(const String& var,const Value& value);
	QRArg& operator=(const String& out) { this->out = out; return *this;}
	vars_t& Vars();
	
	QRArg& SetDefault() { out = Null; return *this; }
	
	Value& GetAdd(const String& var);
	
	bool Is(const String& id) { return this->Id() == id; }
	bool IsDefault() { return IsNull(out); }
	
	const String& Id() { return id;}
	const String& Out() { return out;}
private:
	QtfRapport& that;
	const String& id;
	String out;
};

class QtfRapport
{
public:
	typedef QtfRapport CLASSNAME;
	typedef Gate<QtfRapport&> WhenStart_t;
	typedef Gate<QRArg&> WhenLoop_t;
	typedef Gate<QRArg&> WhenEvaluate_t;
	typedef VectorMap<String, Value> vars_t;
	
	
	QtfRapport(const char* tpl_);
	QtfRapport(const char* tpl_, WhenLoop_t&& _WhenLoop);
	QtfRapport(const char* tpl_, WhenStart_t&& _WhenStart, WhenLoop_t&& _WhenLoop);
	
	QtfRapport& WhenStart(WhenStart_t&& _WhenStart)		{ WhenStart_ = _WhenStart; return *this;}
	QtfRapport& WhenLoop(WhenLoop_t&& _WhenLoop)			{ WhenLoop_ = _WhenLoop; return *this;	}
	QtfRapport& WhenEvaluate(WhenEvaluate_t&& _WhenEvaluate)	{ WhenEvaluate_ = _WhenEvaluate; return *this;	}
	
	RichText	Get();
	int			Print(const Size& page=Size(4500, 6700),const Size& margins=Size(200,200));
	
	operator RichText()								{ return Get(); }
	QtfRapport& operator()(const String& id,const Value& v) {
		Vars().GetAdd(id) = v; return *this;
	}
	
	Value& GetAdd(const String& id) { return Vars().GetAdd(id) ; }
	vars_t& Vars()					{ return _vars;}
private:
	
	WhenStart_t			WhenStart_;
	WhenLoop_t			WhenLoop_;
	WhenEvaluate_t		WhenEvaluate_;
	
	RichTable	TraitTable(const RichTable& orig);
	RichText	TraitTxt(const RichTxt& txt);
	WString		Parse(const WString& ss);
	bool		Evaluate(QRArg& arg);
	
	void		Init();

	
	bool isQtf=false;
	
	vars_t _vars;
	const char* tpl=NULL;
};
#endif
