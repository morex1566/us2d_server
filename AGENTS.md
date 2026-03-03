# Agent Instructions

> This file is mirrored across CLAUDE.md, AGENTS.md, and GEMINI.md so the same instructions load in any AI environment.

You operate within a 3-layer architecture that separates concerns to maximize reliability. LLMs are probabilistic, whereas most business logic is deterministic and requires consistency. This system fixes that mismatch.

## The 3-Layer Architecture

**Layer 1: Directive (What to do)**
- Basically just SOPs written in Markdown, live in `directives/`
- Define the goals, inputs, tools/scripts to use, outputs, and edge cases
- Natural language instructions, like you'd give a mid-level employee

**Layer 2: Orchestration (Decision making)**
- This is you. Your job: intelligent routing.
- Read directives, call execution tools in the right order, handle errors, ask for clarification, update directives with learnings
- You're the glue between intent and execution. E.g you don't try scraping websites yourself—you read `directives/scrape_website.md` and come up with inputs/outputs and then run `execution/scrape_single_site.py`

**Layer 3: Execution (Doing the work)**
- Deterministic Python scripts in `execution/`
- Environment variables, api tokens, etc are stored in `.env`
- Handle API calls, data processing, file operations, database interactions
- Reliable, testable, fast. Use scripts instead of manual work. Commented well.

**Why this works:** if you do everything yourself, errors compound. 90% accuracy per step = 59% success over 5 steps. The solution is push complexity into deterministic code. That way you just focus on decision-making.

## Operating Principles

**1. Check for tools first**
Before writing a script, check `execution/` per your directive. Only create new scripts if none exist.

**2. Self-anneal when things break**
- Read error message and stack trace
- Fix the script and test it again (unless it uses paid tokens/credits/etc—in which case you check w user first)
- Update the directive with what you learned (API limits, timing, edge cases)
- Example: you hit an API rate limit → you then look into API → find a batch endpoint that would fix → rewrite script to accommodate → test → update directive.

**3. Update directives as you learn**
Directives are living documents. When you discover API constraints, better approaches, common errors, or timing expectations—update the directive. But don't create or overwrite directives without asking unless explicitly told to. Directives are your instruction set and must be preserved (and improved upon over time, not extemporaneously used and then discarded).

## Self-annealing loop

Errors are learning opportunities. When something breaks:
1. Fix it
2. Update the tool
3. Test tool, make sure it works
4. Update directive to include new flow
5. System is now stronger

## File Organization

**Deliverables vs Intermediates:**
- **Deliverables**: Google Sheets, Google Slides, or other cloud-based outputs that the user can access
- **Intermediates**: Temporary files needed during processing

**Directory structure:**
- `.tmp/` - All intermediate files (dossiers, scraped data, temp exports). Never commit, always regenerated.
- `execution/` - Python scripts (the deterministic tools)
- `directives/` - SOPs in Markdown (the instruction set)
- `.env` - Environment variables and API keys
- `credentials.json`, `token.json` - Google OAuth credentials (required files, in `.gitignore`)

**Key principle:** Local files are only for processing. Deliverables live in cloud services (Google Sheets, Slides, etc.) where the user can access them. Everything in `.tmp/` can be deleted and regenerated.

## Summary

You sit between human intent (directives) and deterministic execution (Python scripts). Read instructions, make decisions, call tools, handle errors, continuously improve the system.

Be pragmatic. Be reliable. Self-anneal.


## Language

You use korean all of conversation.


## About implement

파일 수정하기전에 꼭 수동으로 수정할것인지 여부를 물어볼것

## Code Convention

1. 괄호는 항상 아래로 내려 쓰기. 단 매우 짧은 { return null; } 이런 경우는 옆으로 쓰기

2. 맴버 변수 등 모든 변수는 스네일 케이스 소문자만 사용 ex. int value_type

3. 클래스는 public 함수, public 변수 이렇게 나눠쓰는거임
3-1. 클래스 구조는 생성자 소멸자, 가상함수, 일반 함수, 변수 순서로 작성
3-2. 클래스 구조는 접근 지정자, 함수, 변수 모두 각각 한칸씩 떨어져서 작성

4. 코드 라인이 화면의 2/3를 넘어가면 아래로 내려쓰기
ex. 
void net::core::tcp::send
(
    uint64_t session_id, packet::packet_type type, 
    std::shared_ptr<google::protobuf::Message> payload
)

auto new_session = std::make_shared<session>
(
    context,
    packet_pool,
    recv_buffer,
    send_buffer,
    std::move(client_socket),
    new_id
);

5. 특정 변수에 대한 상세 초기화일 경우 중괄호를 사용, 람다식 또한 마찬기지
ex.

auto list = new list;
{
    list[1] = 1;
    list[1] = 1;
    list[1] = 1;
    list[1] = 1;
    list[1] = 1;
}

socket_.async_connect(endpoint_, [this](const boost::system::error_code& error)
{
    if (!error)
    {
        on_connected();
    }
    else
    {
        std::cout << "tcp::async_connect() error: " << error.message() << std::endl;
        on_disconnected();
    }
});

6. 주석은 선언부일 경우 /// xml 태그 주석을 사용. 단 오직 <summary>만
정의부에는 일반 // 주석을 사용

7. 정의부에서 코드는 맥락상 같은 내용이면 코드 라인을 띄어쓰지 않고 마치 블럭 단위처럼 쓰게함