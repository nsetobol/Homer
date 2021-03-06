
#include <stdio.h>

#include "JsonParser.h"
#include "ErrorHandler.h"
#include "string.h"

void JSON_Parse(char *json, struct Json_Branch* jBranch, struct Error_Handler *error)
{
	struct JSON_Tokenizer jTokenizer={};
	struct JSON_Token jToken={};
	int dataLen = Strlen(json);
	struct Json_Branch *current=NULL;
	bool isKey=false;
	int cnt=0;

	current = jBranch;

	jTokenizer.at = json;

	for (int i=0;i<dataLen;i++)
	{
		jToken = GetJSONToken(&jTokenizer);

		switch(jToken.type)
		{
			case NEXT:
			{
				if (isKey == true) isKey = false; 
				break;
			}
			case SPACE:
			{
				break;
			}
			case LIST_START:
			{
				struct Json_Branch *realCurrent=NULL;
				realCurrent = current;

				if (isKey == true) 
				{
					current = current->current;
					isKey = false;
				}

				if (current->type == JARRAY)
				{
					Json_AddBranch( current ,JLIST, NULL, NULL);
					current->current->prevBranch = current;
					current = current->current;
					
				}

				current->subBranch = (struct Json_Branch*) Alloc(sizeof(struct Json_Branch));
				current->subBranch->type=JLIST;
				current->subBranch->key=NULL;
				current->subBranch->value=NULL;
				current->subBranch->head=NULL;
				current->subBranch->current=NULL;
				current->subBranch->prevBranch=realCurrent;
				current->subBranch->subBranch=NULL;
				current->subBranch->head=NULL;
				current->subBranch->current=NULL;
				current = current->subBranch;
				//}
				break;
			}
			case ARRAY_START:
			{
				struct Json_Branch *realCurrent=NULL;
				realCurrent = current;

				if (isKey == true) 
				{
					current = current->current;
					isKey = false;
				}

				current->subBranch = (struct Json_Branch*) Alloc(sizeof(struct Json_Branch));
				current->subBranch->type=JARRAY;
				current->subBranch->key=NULL;
				current->subBranch->value=NULL;
				current->subBranch->head= NULL;
				current->subBranch->current=NULL;
				current->subBranch->prevBranch=realCurrent;
				current->subBranch->subBranch=NULL;
				current->subBranch->head=NULL;
				current->subBranch->current=NULL;
				current = current->subBranch;

				break;
			}
			case LIST_END:
			{
				if (isKey == true) isKey = false;
				if (current->prevBranch)
				{
					current = current->prevBranch;
				}
				break;
			}
			case ARRAY_END:
			{
				
				if (isKey == true) isKey = false;

				if (current->prevBranch)
				{
					current = current->prevBranch;
				}
				break;
			}

			case IDENTIFIER:
			{
				char *value = NULL;
				bool isStringLoopDone = false;

				for (int j=0;j < dataLen && isStringLoopDone == false;j++)
				{
					jToken = GetJSONToken(&jTokenizer);

					switch (jToken.type)
					{
						#if 1
						case '\\':
						{
							jToken = GetJSONToken(&jTokenizer);
							break;
						}
						#endif
						case IDENTIFIER:
						{
							isStringLoopDone=true;
							break;
						} 
						default:
						{
							char temp[2]={};
							temp[0] = jToken.token;
							temp[1] = '\0';

							if (value == NULL)
							{
								value = CreateString(temp);
							} else {
								char* tempValue = NULL;

								tempValue = CatString(value,temp);

								Free(value);
								value = NULL;

								value = CreateString(tempValue);
								
								Free(tempValue);
								tempValue=NULL;
							}

							break;
						}
					}
				}
				
				if (value)
				{
					if (isKey==false)
					{
						if (value)
						{
							Json_AddBranch( current ,JDICTIONARY , value, NULL);
							isKey = true;
						}
					} else {
						//assert(current);
						//assert(current->current);
						current->current->value = CreateString(value);
						isKey = false;
					}

					Free(value);
					value = NULL;
				}
				
				break;
			}
		}
	}
}

struct JSON_Token GetJSONToken(struct JSON_Tokenizer *jsonTokenizer)
{
	struct JSON_Token jsonTokenResults={};

	char c=0;
	i8* tokAddr=NULL;

	jsonTokenResults.length = 1;
	jsonTokenResults.JsonData = jsonTokenizer->at;

	c = jsonTokenizer->at[0];
	jsonTokenResults.token = c;
	if (c != '\0')
	{
		++jsonTokenizer->at;
	}

	switch(c)
	{
		case 0x20:
		{
			jsonTokenResults.type = SPACE;
			break;
		}
		case '{':
		{
			jsonTokenResults.type = LIST_START;
			break;
		}
		case '}':
		{
			jsonTokenResults.type = LIST_END;
			break;
		}
		case '[':
		{
			jsonTokenResults.type = ARRAY_START;
			break;
		}
		case ']':
		{
			jsonTokenResults.type = ARRAY_END;
			break;
		}
		case '"':
		{
			jsonTokenResults.type = IDENTIFIER;
			break;
		}
		case ':':
		{
			jsonTokenResults.type = SPLITTER;
			break;
		}
		case ',':
		{
			jsonTokenResults.type = NEXT;
			break;
		}

	};

	return jsonTokenResults;
}

void JSON_AddBranch(struct JSON_Tree *jsonTree, int type,  char* text)
{
	if (jsonTree->root ==NULL)
	{
		jsonTree->root = (struct JSON_Branch*) Alloc(sizeof(struct JSON_Branch));
		assert(jsonTree->root);
		jsonTree->root->prev= NULL;
		jsonTree->root->type= type;
		jsonTree->root->text= text;
		jsonTree->root->value= NULL;
		jsonTree->root->next = NULL;
		jsonTree->root->subTree = NULL;
		jsonTree->current = jsonTree->root;
	} else {
		assert(jsonTree->current);

		jsonTree->current->next = (struct JSON_Branch*) Alloc(sizeof(struct JSON_Branch));

		assert(jsonTree->current->next);

		jsonTree->current->next->type= type;
		jsonTree->current->next->text = text;
		jsonTree->current->next->value = NULL;
		jsonTree->current->next->next = NULL;
		jsonTree->current->next->subTree = NULL;
		jsonTree->current->next->prev= jsonTree->current;
		jsonTree->current = jsonTree->current->next;
	}
}

void JSON_AddTree(struct JSON_Branch* branch, int type)
{
	branch->subTree= (struct JSON_Tree*) Alloc(sizeof(struct JSON_Tree));

	assert(branch->subTree);

	branch->subTree->type = type;
	branch->subTree->root = NULL;
	branch->subTree->current = NULL;
	branch->subTree->prevTree = NULL;
	branch->subTree->subTree=NULL;
}

void JSON_AddTree2(struct JSON_Tree* tree, int type)
{
	tree->subTree= (struct JSON_Tree*) Alloc(sizeof(struct JSON_Tree));
	
	assert(tree->subTree);

	tree->subTree->type = type;
	tree->subTree->root = NULL;
	tree->subTree->prevTree = NULL;
	tree->subTree->current = NULL;
	tree->subTree->subTree=NULL;
}


struct JSON_Branch *GetCurrentBranch(struct JSON_Tree* tree)
{
	for (struct JSON_Branch* iter=tree->root;iter != NULL;iter = iter->next)
	{
		if (iter->next == NULL)
		{
			return iter;
		}
	}
	return NULL;
}

struct Json_Branch *Json_DelTree(Json_Branch* branch)
{
	Json_Branch *retTree=NULL;
	if (branch->subBranch)
	{
		retTree=  Json_DelTree(branch->subBranch);

	} 
	if (branch->head)
	{
		struct Json_Branch *retBranch = Json_DelBranch(branch->head);

		if (retBranch != NULL)
		{
			if (retBranch->key)
			{
				//printf("Deleted key: %s\n", retBranch->key);
				Free(retBranch->key);
				retBranch->key=NULL;
			}
			if (retBranch->value)
			{
				//printf("Deleted value: %s\n", retBranch->value);

				Free(retBranch->value);
				retBranch->value=NULL;
			}
			Free(retBranch);
			retBranch=NULL;
		}
	}

	if (retTree!= NULL)
	{
		if (retTree->key)
		{

			//printf("Deleted key: %s\n", retTree->key);
			Free(retTree->key);
			retTree->key=NULL;
		}
		if (retTree->value)
		{
			//printf("Deleted value: %s\n", retTree->value);
			Free(retTree->value);
			retTree->value=NULL;
		}

		//printf("Deleted Tree\n");
		Free(retTree);
		retTree=NULL;
	}

	return branch;
}

struct Json_Branch* Json_DelBranch(Json_Branch* branch)
{
	Json_Branch *retTree=NULL;

	if (branch->subBranch)
	{
		retTree=  Json_DelTree(branch->subBranch);
	}
	if (branch->next)
	{
		Json_Branch *retBranch = Json_DelBranch(branch->next);

		if (retBranch != NULL)
		{
			if (retBranch->key)
			{
				//printf("Deleted key: %s\n", retBranch->key);
				Free(retBranch->key);
				retBranch->key=NULL;
			}
			if (retBranch->value)
			{
				//printf("Deleted value: %s\n", retBranch->value);
				Free(retBranch->value);
				retBranch->value=NULL;
			}
			Free(retBranch);
			retBranch=NULL;
		}
	}

	if (retTree!= NULL)
	{
		if (retTree->key)
		{
			//printf("Deleted key: %s\n", retTree->key);
			Free(retTree->key);
			retTree->key=NULL;
		}
		if (retTree->value)
		{
			//printf("Deleted value: %s\n", retTree->value);
			Free(retTree->value);
			retTree->value=NULL;
		}
		Free(retTree);
		retTree=NULL;
	}
	return branch;
}

struct Json_Branch* Json_GetJsonNodeFromKey(struct Json_Branch* branch, struct array* list)
{
	struct Json_Branch *resultBranch = NULL;
	struct Json_Branch *branchStart = NULL;
	struct Json_Branch *branchHead= NULL;
	bool foundFirst = false;
	int listIndex = 0;

	branchStart = branch;
	branchHead = branch->head;

	while (branch != NULL)
	{
		if (branchHead == NULL)
		{
			if (branch)
			{
				if (branch->subBranch)
				{
					branchHead = branch->subBranch->head;
					branch = branch->subBranch;
				} else {
					//break;
				}
			}
		} else {
			if (branchHead)
			{
				if (branchHead->key != NULL)
				{
					char *gotFromList = (char*) GetFromArray(list,listIndex);
					bool foundKey = false;

					if (gotFromList)
					{
						//printf("comparing %s with %s\n", branchHead->key,gotFromList);
						if (strcmp(gotFromList, branchHead->key) == 0)
						{
							if (foundFirst == false)
							{
								foundFirst = true;
							}
							foundKey = true;	
							printf("found %s\n", branchHead->key);
							
							resultBranch = branchHead;

							if (branchHead->subBranch)
							{
								if (branchHead->subBranch->type == JARRAY)
								{
									branch = branchHead->subBranch;
									branchHead = branch->head;

									if (branchHead)
									{
										branch = branchHead->subBranch;

										if (branch)
										{
											branchHead = branch->head;

										}
									}

								} else{
									branch = branchHead->subBranch;
									branchHead = branch->head;

								}
							} else {
								if (branch->prevBranch)
								{
									if (branch->prevBranch->head)
									{
										printf("prev branch type is %i\n", branch->prevBranch->head->type);
									}
								}
								break;
							}
#if 0
							if (branchHead->subBranch)
							{
								if (branchHead->subBranch->head)
								{
									branch = branchHead->subBranch;
									branchHead = branchHead->subBranch->head;
									

								}
							}
							#endif
							listIndex++;
						
						} else {
							if (foundFirst == true)
							{
								//break;
							}
						}
					} else {
						printf("no more items on the list\n");
					}
					if (foundKey == false)
					{
						if (branchHead->next)
						{
							branchHead = branchHead->next;
						} else {
							//branch = branch->prevBranch;
							//branchHead = branch->current;
							break;
						}
					
					}

				} 
				
				if (branch->head == NULL && branch->subBranch == NULL)
				{
					break;
				}

			}
		}
	}
	return resultBranch;
}


void Json_Free(Json_Branch* branch)
{
	Json_DelTree(branch);
}

void Json_AddBranch(struct Json_Branch* branch, struct Json_Type type, char* key, char* value)
{
	if (branch->head == NULL)
	{
		branch->head = (struct Json_Branch*) Alloc(sizeof(struct Json_Branch));
		branch->head->type = type;
		if (key != NULL)
		{
			branch->head->key = CreateString(key);
		} else {
			branch->head->key = NULL;
		}

		if (value != NULL)
		{
			branch->head->value= CreateString(value);
		} else {
			branch->head->value=NULL;
		}
		branch->head->next= NULL;
		branch->head->prev= NULL;
		branch->head->head= NULL;
		branch->head->current= NULL;
		branch->current = branch->head;
	} else {
		branch->current->next = (struct Json_Branch*) Alloc(sizeof(struct Json_Branch));
		branch->current->next->type = type;
		if (key)
		{
			branch->current->next->key = CreateString(key);
		} else {
			branch->current->next->key = NULL;
		}
		if (value)
		{
			branch->current->next->value= CreateString(value);;
		} else {
			branch->current->next->value= NULL;;
		}
		branch->current->next->prev= branch->current;
		branch->current->next->head= NULL;
		branch->current->next->current= NULL;
		branch->current = branch->current->next;
	}
}

