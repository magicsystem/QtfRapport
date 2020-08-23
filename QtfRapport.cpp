#include "QtfRapport.h"

QRArg& QRArg::operator()(const String& var,const Value& value)
{
	GetAdd(var)=value;
	return *this;
}

QtfRapport::vars_t& QRArg::Vars()
{
	return that.Vars();
}

Value& QRArg::GetAdd(const String& var)
{
	return that.GetAdd(var);
}





QtfRapport::QtfRapport(const char* tpl_) : tpl(tpl_)
{
	WhenStart_ = [](QtfRapport&) { return true; };
	WhenLoop_ = [] (QRArg&) -> bool { return false; };
	Init();
}

QtfRapport::QtfRapport(const char* tpl_, WhenLoop_t&& _WhenLoop): tpl(tpl_)
{
	WhenStart_ = [](QtfRapport&) { return true; };
	WhenLoop_ = pick(_WhenLoop);
	Init();
}

QtfRapport::QtfRapport(const char* tpl_, WhenStart_t&& _WhenStart, WhenLoop_t&& _WhenLoop): tpl(tpl_)
{
	WhenStart_ = pick(_WhenStart);
	WhenLoop_ = pick(_WhenLoop);
	Init();
}

void QtfRapport::Init() {
	WhenEvaluate_ = [this](QRArg& arg) -> bool {
		String out;
		Vector<String> parts = Split(arg.Id(), " ", true);
		int ii;
		for(int i=0; i<parts.GetCount(); i++)
		{
			ii = _vars.Find(AsString(parts[i]));
			if(ii >= 0)
			{
				out << AsString (Vars()[ii]);
			}
			
			else
			{
				out << parts[i];
			}
		}
		arg = pick(out);
		
		return true;
	};
}

RichText QtfRapport::Get()
{
	RichText txt,result;
	if(tpl == NULL) return result;
	
	if(ParseQTF(txt, tpl) /*&& ParseQTF(result, tpl)*/ )
	{
		if(WhenStart_(*this)) {
			//return result;
			result = pick(TraitTxt(txt));
			
			RichText tmp;
			
			if(ParseQTF(tmp,txt.GetHeaderQtf()))
			{
				result.SetHeaderQtf( AsQTF( TraitTxt(tmp) ));
			}
			
			tmp.Clear();
			if(ParseQTF(tmp,txt.GetFooterQtf()))
			{
				result.SetFooterQtf( AsQTF( TraitTxt(tmp) ));
			}
			
			//result.SetHeaderQtf( Parse(txt.GetHeaderQtf()) );
			//result.SetHeaderQtf(txt.GetHeaderQtf());
			//result.SetFooterQtf(txt.GetFooterQtf());
			
		}
	}
	return result;
}

int QtfRapport::Print(const Size& page,const Size& margin)
{
	Report r;
	r.SetPageSize(page);
	r.Margins(margin.cx,margin.cy);
	
	r.Put(Get());
	
	ReportWindow rw;
	rw.Zoomable().Sizeable();
	
	return rw.Perform(r);
}


RichTable QtfRapport::TraitTable(const RichTable& orig)
{
	VectorMap<String, int> pos;
	// find all template rows
	
	for(int r=0; r<orig.GetRows(); r++)
	{
		bool break_ = false;
		WString id;
		for(int c=0; c<orig.GetColumns(); c++)
		{
			const RichTxt& rt = orig.Get(r,c);
			for(int p=0; p<rt.GetPartCount(); p++)
			{
				if(rt.IsPara(p)) {
					const WString& plt =  rt.GetPlainText();
					if(plt.StartsWith("[]")) {
						const wchar *b = plt.Begin() + 2;
						const wchar *e = plt.End();
						
						
						if(b[0] != L' ' && b[0] != L'\t' && b[0] != L'\n')
						{
							wchar *pch = (wchar*) b;
							pch++;
							while(pch != e)
							{
								if(pch[0] == L' ' || pch[0] == L'\t' || pch[0] == L'\n')
								{
									id=WString(b,pch);
									break;
								}
								pch++;
							}
						}
						
						break_ = true;
						break;
					}
				}
			}
			
			if(break_) break;
		}
		
		if(break_) {
			String uid = id.ToString();
			int ii = pos.Find(uid);
			if(ii<0) pos.Add(uid,r);
			else throw Exc("Duplicate row");
		}
	} // rows
	
	// noting to do.
	if(pos.GetCount() == 0)
	{
		RichTable tab = clone(orig);
		int ro,rr;
		ro = 0;
		rr = 0;
		while(ro < orig.GetRows())
		{
			for(int c=0; c<orig.GetColumns(); c++) {
				tab.SetPick(rr,c, pick( TraitTxt(orig.Get(ro,c) )));
				tab.SetFormat(rr,c,orig.GetFormat(ro,c));
			}
			rr++;ro++;
		}
		
		return tab;
	}
	
	// work in clone.
	else
	{
		RichTable tab = clone(orig);
		int fr = pos[0],ro;
		
		// remove element antell first template row
		int rr = tab.GetRows() - 1;
		while(rr > fr)
		{
			tab.RemoveRow(rr);
			rr--;
		}
		
		rr = fr;
		
		// render & add rows.
		QRArg arg_(*this, pos.GetKey(0));
		
		while( WhenLoop_(arg_) )
		{
			if(!arg_.IsDefault())
			{
				fr = pos.Find(arg_.Out());
				if(fr<0) throw Exc(Format("template row not found: %s",arg_.Out()));
				ro = pos[fr];
			}
			
			else
			{
				// set first one as default.
				ro = pos[0];
			}
			
			Size sz;
			for(int c=0; c<orig.GetColumns(); c++) {
				tab.SetPick(rr,c, pick( TraitTxt(orig.Get(ro,c) )));
				tab.SetFormat(rr,c,orig.GetFormat(ro,c));
				
				sz = orig.GetSpan(ro,c);
				tab.SetSpan(rr,c,sz.cy,sz.cx);
			}
			
			
			
			rr++;
		}
		
		// copy other rows to finish the table.
		ro = pos[pos.GetCount() - 1 ] + 1;
		while(ro < orig.GetRows())
		{
			for(int c=0; c<orig.GetColumns(); c++) {
				tab.SetPick(rr,c, pick( TraitTxt(orig.Get(ro,c) )));
				tab.SetFormat(rr,c,orig.GetFormat(ro,c));
			}
			rr++;ro++;
		}
		
		return tab;
	} // if(pos.GetCount() == 0)
	
	
	
	/* / apply
	for(int r=0; r<orig.GetRows(); r++)
	{
		bool break_ = false;
		WString id;
		for(int c=0; c<orig.GetColumns(); c++)
		{
			const RichTxt& rt = orig.Get(r,c);
			for(int p=0; p<rt.GetPartCount(); p++)
			{
				if(rt.IsPara(p)) {
					const WString& plt =  rt.GetPlainText();
					if(plt.StartsWith("[]")) {
						const wchar *b = plt.Begin() + 2;
						const wchar *e = plt.End();
						
						
						if(b[0] != L' ' && b[0] != L'\t' && b[0] != L'\n')
						{
							wchar *pch = (wchar*) b;
							pch++;
							while(pch != e)
							{
								if(pch[0] == L' ' || pch[0] == L'\t' || pch[0] == L'\n')
								{
									id=WString(b,pch);
									break;
								}
								pch++;
							}
						}
						
						break_ = true;
						break;
					}
				}
			}
			
			if(break_) break;
		}
		
		if(break_) {
			int rr;
			if(rfind < 0)
			{
				rfind = r;
				rr = tab.GetRows() - 1;
				
				while( rr >= r ) {
					tab.RemoveRow(rr); rr --;
				}
			}
			
			rr = tab.GetRows();
			QRArg arg_(*this,id.ToString());
			
			while( WhenLoop_(arg_) )
			{
				for(int c=0; c<tab.GetColumns(); c++) {
					tab.SetPick(rr,c, pick( TraitTxt(orig.Get(r,c) )));
				}
				rr++;
			}
			
		} else {
			// after row /#
			if(rfind >= 0 )
			{
				int rr = tab.GetRows();
				for(int c=0; c<tab.GetColumns(); c++) {
					tab.SetPick(rr,c, pick( TraitTxt(orig.Get(r,c) )));
				}
			}
			
			// before row /#
			else
			{
				int rr = r;
				for(int c=0; c<tab.GetColumns(); c++) {
					tab.SetPick(rr,c, pick( TraitTxt(orig.Get(r,c) )));
				}
			}
		}
	}
	
	return tab;*/
}

RichText QtfRapport::TraitTxt(const RichTxt& txt)
{
	RichText result;
	for(int i=0; i<txt.GetPartCount(); i++)
	{
		// table
		if(txt.IsTable(i))
		{
			result.CatPick(pick(TraitTable(txt.GetTable(i))));
		}
		
		// text
		else if(txt.IsPara(i))
		{
			// trait para
			RichPara para = txt.Get(i,RichStyle::GetDefault());
			
			for(int pi=0; pi<para.GetCount(); pi++)
			{
				if(para[pi].IsText())
				{
					para[pi].text = Parse(para[pi].text);
				}
			}
			
			result.Cat(para);
		}
	}
	
	return result;
}

WString QtfRapport::Parse(const WString& ss)
{
	WString out;
	const wchar* b = ss.Begin();
	const wchar* e = ss.End();
	wchar *p,*pb;
	wchar p0=0,p1=0,p2=0;
	bool isOpen = false;
	
	p = (wchar*) b ;
	pb = p;
	
	while(p != e) {
		p0=p[0];
		
		if(p2 != L'$' && p1==L'$' && p0==L'{')
		{
			out << WString(pb, p-1);
			pb = p + 1;
			isOpen = true;
		} else
			
		if(p2 != L'/' && p1==L'[' && p0==L']')
		{
			while(p != b && p[0] != L' ' && p[0] != L'\t' && p[0] != L'\n') p++;
			
			pb = p + 1;
		} else
			
		if(p1!=L'}' && p0==L'}' && isOpen)
		{
			QRArg arg(*this, WString(pb, p).ToString() );
			if(Evaluate(arg))
			{
				out << arg.Out().ToWString();
			}
			pb = p + 1;
		}
		
		p2=p1;p1=p0;
		p++;
	}
	
	if(pb != e) {
		out << WString(pb, e) ;
	}
	
	return out;
}

bool QtfRapport::Evaluate(QRArg& arg)
{
	return WhenEvaluate_(arg);
}

