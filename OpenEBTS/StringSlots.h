#ifndef STRINGSLOTS_H
#define STRINGSLOTS_H

// Simple class to manage allocation of multiple strings in memory
// to be passed as out parameters to many functions.

class CStringSlots
{
public:
	CStringSlots() {}

	~CStringSlots()
	{
		// Delete all strings in string array

		size_t nSize = m_rgszStrings.size();
		TCHAR* sz = NULL;

		for (unsigned int i = 0; i < nSize; i++)
		{
			sz = m_rgszStrings.at(i);
			if (sz) delete sz;
		}
		m_rgszStrings.clear();
	}

	TCHAR* AddNew(CStdString s)
	{
		TCHAR *szNew = new TCHAR[s.GetLength() + 1];
		_tcscpy(szNew, s);

		// Add to our managed array of string slots, and return pointer
		m_rgszStrings.push_back(szNew);

		return szNew;
	}

private:
	std::vector<TCHAR*> m_rgszStrings;
};

#endif // #ifndef STRINGSLOTS_H
