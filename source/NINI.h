#pragma once

/*

N-INI by Naseband

Simple INI loader. Not beautiful, but it works.

*/

#include <iostream>
#include <cstdlib>
#include <vector>

// ------------------------------------------------------------ 

class NINI 
{
private:

	struct sNINITag
	{
		char 
			*szName = NULL;
		int someval = 5;
	};

	struct sNINIKey
	{
		bool
			bText = false;

		char 
			*szName = NULL,
			*szValue = NULL,
			*szText = NULL;
		
		int
			nTag = -1;
	};

	bool m_bLoaded = false;
	DWORD m_dwLoadTime = 0;
	std::vector<sNINIKey> m_vecKeys;
	std::vector<sNINITag> m_vecTags;

	bool AddKey(char** name, char** value, int tag)
	{
		int len = 0;
		sNINIKey tmpkey;

		// Name

		while ((*name)[len] != 0)
			++len;

		if (len == 0)
			return false;

		tmpkey.szName = *name;

		// Value

		len = 0;

		while ((*value)[len] != 0)
			++len;

		if (len == 0)
			return false;

		tmpkey.szValue = *value;

		// Tag

		tmpkey.nTag = tag;

		// Fin

		m_vecKeys.push_back(tmpkey);

		return true;
	}

	int AddTag(char** name)
	{
		int 
			len = 0;
		sNINITag 
			tmptag;
		size_t
			i, s;

		while ((*name)[len] != 0)
			++len;

		if (len == 0)
			return -1;

		for (i = 0, s = m_vecTags.size(); i < s; ++i)
			if (CmpString(m_vecTags.at(i).szName, *name) == 0)
				return (int)i;

		tmptag.szName = *name;
		tmptag.someval = 27;

		m_vecTags.push_back(tmptag);

		return (int)(m_vecTags.size() - 1U);
	}

	bool AddText(char** text)
	{
		sNINIKey tmpkey;

		tmpkey.bText = true;
		tmpkey.szText = *text;

		m_vecKeys.push_back(tmpkey);

		return true;
	}

public:
	NINI()
	{

	}

	~NINI()
	{
		Unload();
	}

	bool Load(const char* filename, bool store_texts = true)
	{
		if (m_bLoaded)
			Unload();

		DWORD
			tick = GetTickCount();

		FILE
			*pFile;
		errno_t 
			err;

		err = fopen_s(&pFile, filename, "r");

		if (err != 0)
			return false;
		
		int c_i;
		char c;

		int cur_size = 100;
		int cur_len = 0;
		int seperator_pos = -1;
		int tag_pos1 = -1;
		int len;
		int i;
		int tag = -1;
		bool skip_cur_line = false;
		bool only_spaces = true;
		bool is_tag = false;

		char* line = (char*)malloc(cur_size);

		if (line == NULL)
		{
			fclose(pFile);
			return false;
		}

		while ((c_i = fgetc(pFile)) >= EOF)
		{
			c = (char)c_i;

			if (c == '\r')
				continue;

			if (c == '#' && only_spaces) // comment, skip this
				skip_cur_line = true;

			if (c == '[' && only_spaces && tag_pos1 == -1)
				tag_pos1 = cur_len;

			if (c != ' ' && only_spaces) // End pad
				only_spaces = false;

			if (c_i == -1 || c == '\n') // End of line (and/or file)
			{
				if (cur_len >= 3 && seperator_pos != -1 && !skip_cur_line) // A valid value
				{
					char* name = (char*)malloc(size_t(seperator_pos) + 1U);

					if (name != NULL)
					{
						char* value = (char*)malloc(size_t(cur_len) + 1 - size_t(seperator_pos));

						if (value != NULL)
						{
							len = 0;

							for (i = 0; i < seperator_pos; ++i)
							{
								name[i] = line[i];
								++len;
							}
							name[len] = 0;

							TrimText(name);

							len = 0;

							for (i = 0; i < cur_len - (seperator_pos + 1); ++i)
							{
								value[i] = line[seperator_pos + 1 + i];
								++len;
							}
							value[len] = 0;

							TrimText(value);

							if (AddKey(&name, &value, tag))
							{
								name = NULL; // Pointers are now stored in the vector, don't free them here
								value = NULL;
							}
							
							if (value != NULL)
								free(value);
						}

						if(name != NULL)
							free(name);
					}
				}
				else if (cur_len == 0 || (skip_cur_line && !is_tag)) // Invalid value, no tag as well - text
				{
					char* text = (char*)malloc(1 + size_t(cur_len));

					if (text != NULL)
					{
						for (i = 0; i < cur_len; ++i)
						{
							text[i] = line[i];
						}
						text[cur_len] = 0;

						if (!AddText(&text))
							free(text);
					}
				}

				seperator_pos = -1;
				cur_len = 0;
				skip_cur_line = false;
				only_spaces = true;
				tag_pos1 = -1;
				is_tag = false;
			}
			else
			{
				if (!skip_cur_line)
				{
					if (seperator_pos == -1 && c == '=') // Seperator, this is a potential key
					{
						only_spaces = true;
						for (i = 0; i < cur_len; ++i)
							if (line[i] != ' ')
								only_spaces = false;

						if (only_spaces)
						{
							skip_cur_line = true;
							continue;
						}
						else
						{
							seperator_pos = cur_len;
						}
					}
				}

				line[cur_len++] = c;
				line[cur_len] = 0;

				if (cur_len >= cur_size - 2) // Resize buffer
				{
					char* tmp = (char*)realloc(line, cur_size * 2);

					if (tmp == NULL)
					{
						skip_cur_line = true;
					}
					else
					{
						line = tmp;
						cur_size *= 2;
					}
				}

				if (!skip_cur_line && c == ']' && tag_pos1 != -1) // Check for Tag
				{
					len = (cur_len - 1) - (tag_pos1 + 1);

					if (len <= 0)
					{
						tag = -1;
					}
					else
					{
						char* tmp = (char*)malloc(len + 1);

						if (tmp != NULL)
						{
							for (i = 0; i < len; ++i)
								tmp[i] = line[tag_pos1 + 1 + i];

							tmp[len] = 0;

							tag = AddTag(&tmp);
							is_tag = true;
						}
						else
						{
							tag = -1;
						}
					}

					skip_cur_line = true;
				}
			}

			if (c_i == -1)
				break;
		}
		
		free(line);

		fclose(pFile);

		m_dwLoadTime = GetTickCount() - tick;

		m_bLoaded = true;

		return true;
	}

	bool Unload()
	{
		if (!m_bLoaded)
			return false;

		for (size_t v = 0, s = m_vecKeys.size(); v < s; ++v)
		{
			if (m_vecKeys.at(v).szName != NULL)
				free(m_vecKeys.at(v).szName);

			if (m_vecKeys.at(v).szValue != NULL)
				free(m_vecKeys.at(v).szValue);

			if (m_vecKeys.at(v).szText != NULL)
				free(m_vecKeys.at(v).szText);
		}

		m_vecKeys.clear();

		for (size_t v = 0, s = m_vecTags.size(); v < s; ++v)
		{
			if (m_vecTags.at(v).szName != NULL)
				free(m_vecTags.at(v).szName);
		}

		m_vecTags.clear();

		m_bLoaded = false;

		return true;
	}

	bool IsLoaded()
	{
		return m_bLoaded;
	}

	bool Write(char* filename, bool clean = false)
	{
		FILE
			*pFile;
		errno_t
			err;

		err = fopen_s(&pFile, filename, "w");

		if (err != 0)
			return false;

		size_t
			i,
			count = m_vecKeys.size(),
			lines = 0;

		sNINIKey
			*kptr;

		sNINITag
			*tptr;

		int
			last_tag = -1,
			cur_tag = -1;

		for (i = 0; i < count; ++i)
		{
			kptr = &m_vecKeys.at(i);

			if (clean && kptr->bText)
				continue;

			cur_tag = kptr->nTag;

			if (kptr->bText)
			{
				fprintf(pFile, "%s\n", kptr->szText);
				++lines;
			}
			else
			{
				if (last_tag != cur_tag)
				{
					if (cur_tag == -1)
					{
						if(clean && lines != 0)
							fprintf(pFile, "\n[]\n");
						else
							fprintf(pFile, "[]\n");
						++lines;
					}
					else
					{
						tptr = &m_vecTags[cur_tag];

						if (clean && lines != 0)
							fprintf(pFile, "\n[%s]\n", tptr->szName);
						else
							fprintf(pFile, "[%s]\n", tptr->szName);
						++lines;
					}

					last_tag = cur_tag;
				}
				fprintf(pFile, "%s = %s\n", kptr->szName, kptr->szValue);
				++lines;
			}
		}

		fclose(pFile);

		return true;
	}

	DWORD GetLoadTime()
	{
		return m_dwLoadTime;
	}

	// ----------------------------------------------------------

	int CountKeys()
	{
		int count = 0;

		for (size_t i = 0, s = m_vecKeys.size(); i < s; ++i)
			if (!m_vecKeys.at(i).bText)
				++count;

		return count;
	}

	int CountTexts()
	{
		int count = 0;

		for (size_t i = 0, s = m_vecKeys.size(); i < s; ++i)
			if (m_vecKeys.at(i).bText)
				++count;

		return count;
	}

	bool GetKeyName(int id, char* dest, size_t size)
	{
		int count = int(m_vecKeys.size());

		if (id < 0 || id >= count || m_vecKeys.at(id).bText)
			return false;

		dest[0] = 0;
		strcat_s(dest, size, m_vecKeys.at((size_t)id).szName);

		return true;
	}

	bool GetKeyValue(int id, char* dest, size_t size)
	{
		int count = int(m_vecKeys.size());

		if (id < 0 || id >= count || m_vecKeys.at(id).bText)
			return false;

		dest[0] = 0;
		strcat_s(dest, size, m_vecKeys.at((size_t)id).szValue);

		return true;
	}

	bool GetKeyTag(int id, int &tag)
	{
		int count = int(m_vecKeys.size());

		if (id < 0 || id >= count || m_vecKeys.at(id).bText)
			return false;

		tag = m_vecKeys.at(id).nTag;

		return true;
	}

	int FindKeyByName(const char* name, const char* tag = "", int offset = 0)
	{
		for (size_t v = 0, s = m_vecKeys.size(); v < s; ++v)
		{
			if (!m_vecKeys.at(v).bText && CmpString(m_vecKeys.at(v).szName, name) == 0 && (tag[0] == 0 || (m_vecKeys.at(v).nTag != -1 && CmpString(m_vecTags.at(m_vecKeys.at(v).nTag).szName, tag) == 0)))
			{
				if(offset-- == 0)
					return static_cast<int>(v);
			}
		}

		return -1;
	}

	bool GetKeyValueByName(const char* name, char* dest, size_t size, const char* tag = "")
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		dest[0] = 0;
		strcat_s(dest, size, m_vecKeys.at(id).szValue);

		return true;
	}

	bool GetKeyValueByName(const char* name, int &dest, const char* tag = "")
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		char value[30] = { 0 };

		strcat_s(value, 30, m_vecKeys.at(id).szValue);

		if (sscanf_s(value, "%d", &dest) != 1)
			return false;

		return true;
	}

	/*bool GetKeyValueByName(const char* name, char &dest, char* tag = nullptr)
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		char value[30] = { 0 };

		strcat_s(value, 30, m_vecKeys.at(id).szValue);

		if (sscanf_s(value, "%d", &dest) != 1)
			return false;

		return true;
	}*/

#if defined WIN32
	bool GetKeyValueByName(const char* name, DWORD& dest, const char* tag = "")
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		char value[30] = { 0 };
		int tmp;

		strcat_s(value, 30, m_vecKeys.at(id).szValue);

		if (sscanf_s(value, "%d", &tmp) != 1)
			return false;

		dest = static_cast<DWORD>(tmp);

		return true;
	}
#endif

	bool GetKeyValueByName(const char* name, float& dest, const char* tag = "")
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		char value[30] = { 0 };

		strcat_s(value, 30, m_vecKeys.at(id).szValue);

		if (sscanf_s(value, "%f", &dest) != 1)
			return false;

		return true;
	}

	bool GetKeyValueByName(const char* name, double& dest, const char* tag = "")
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		char value[30] = { 0 };

		strcat_s(value, 30, m_vecKeys.at(id).szValue);

		if (sscanf_s(value, "%lf", &dest) != 1)
			return false;

		return true;
	}

	bool GetKeyValueByName(const char* name, bool& dest, const char* tag = "")
	{
		int id = FindKeyByName(name, tag);

		if (id == -1)
			return false;

		char value[30] = { 0 };
		int val_i = 0;

		strcat_s(value, 30, m_vecKeys.at(id).szValue);

		if (CmpString(value, "true") == 0)
		{
			dest = true;
			return true;
		}
		if (CmpString(value, "false") == 0)
		{
			dest = false;
			return true;
		}
		else if (sscanf_s(value, "%d", &val_i) != 1)
		{
			return false;
		}

		dest = (val_i != 0);

		return true;
	}

	// ----------------------------------------------------------

	int CountTags()
	{
		return int(m_vecTags.size());
	}

	bool GetTag(int id, char* dest, size_t size)
	{
		if (id < 0 || id >= int(m_vecTags.size()))
			return false;

		dest[0] = 0;
		strcat_s(dest, size, m_vecTags.at(id).szName);

		return true;
	}

	// ----------------------------------------------------------

	void GetTextPad(char* text, int& first_char, int& last_char)
	{
		size_t
			len = strlen(text),
			i;

		first_char = -1;
		last_char = -1;

		for (i = 0; i < len; ++i)
		{
			if (text[i] != ' ')
			{
				if (first_char == -1)
					first_char = int(i);

				last_char = int(i);
			}
		}
	}

	void TrimText(char* text)
	{
		int
			first_char,
			last_char;
		GetTextPad(text, first_char, last_char);

		if (first_char == -1)
		{
			text[0] = 0;
			return;
		}

		size_t 
			a = first_char,
			b = 0,
			len = last_char + 1 - first_char;

		while (b < len)
			text[b++] = text[a++];

		text[len] = 0;
	}

	int CmpString(const char* string1, const char* string2, bool ignore_case = true) 
	{
		if (ignore_case)
		{
			int i;

			for (i = 0; string1[i] && string2[i]; ++i)
			{
				if (string1[i] == string2[i] || (string1[i] ^ 32) == string2[i])
					continue;
				else
					break;
			}

			if (string1[i] == string2[i])
				return 0;
			if ((string1[i] | 32) < (string2[i] | 32))
				return -1;

			return 1;
		}

		return strcmp(string1, string2);
	}
};

// ------------------------------------------------------------  EOF
