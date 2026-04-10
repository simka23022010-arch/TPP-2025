sharp
using System;
using System.Collections.Generic;
using System.Linq;

namespace ResearchComplexGame
{
    #region 2.1 Интерфейсы
    public interface ICommand
    {
        void Execute(GameState state);
    }

    public interface IInteractable
    {
        string Name { get; }
        void Interact(Player player, GameState state);
    }

    public interface ICondition
    {
        bool Check(GameState state);
    }

    public interface IEffect
    {
        void Apply(GameState state);
    }
    #endregion

    #region 2.2 Абстрактные классы
    public abstract class CommandBase : ICommand
    {
        public abstract void Execute(GameState state);
    }

    public abstract class ConditionBase : ICondition
    {
        public abstract bool Check(GameState state);
    }

    public abstract class EffectBase : IEffect
    {
        public abstract void Apply(GameState state);
    }

    public abstract class GameEventBase
    {
        public List<ICondition> Conditions { get; set; } = new List<ICondition>();
        public List<IEffect> Effects { get; set; } = new List<IEffect>();

        public virtual void TryTrigger(GameState state)
        {
            if (Conditions.All(c => c.Check(state)))
            {
                foreach (var effect in Effects) effect.Apply(state);
            }
        }
    }

    public abstract class InteractableObject : IInteractable
    {
        public string Name { get; protected set; }
        public List<ICondition> Conditions { get; set; } = new List<ICondition>();
        public List<IEffect> InteractionEffects { get; set; } = new List<IEffect>();

        public virtual void Interact(Player player, GameState state)
        {
            if (Conditions.All(c => c.Check(state)))
            {
                foreach (var effect in InteractionEffects) effect.Apply(state);
            }
            else
            {
                state.Log("Условия не выполнены для взаимодействия с этим объектом.", ConsoleColor.Yellow);
            }
        }
    }
    #endregion

    #region 2.3 Основные классы (Ядро)
    public class Player
    {
        public int Health { get; set; } = 100;
        public List<string> Inventory { get; } = new List<string>();
    }

    public class GameState
    {
        public Player Player { get; } = new Player();
        public Location CurrentLocation { get; set; }
        public Dictionary<string, Location> World { get; } = new Dictionary<string, Location>();
        public Dictionary<string, bool> Flags { get; } = new Dictionary<string, bool>();
        public List<Quest> Quests { get; } = new List<Quest>();
        public bool IsRunning { get; set; } = true;

        public void Log(string message, ConsoleColor color = ConsoleColor.Gray)
        {
            Console.ForegroundColor = color;
            Console.WriteLine(message);
            Console.ResetColor();
        }
    }

    public class Location
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public Dictionary<string, string> Exits { get; } = new Dictionary<string, string>();
        public List<InteractableObject> Objects { get; } = new List<InteractableObject>();
        public List<GameEventBase> Events { get; } = new List<GameEventBase>();

        public void AddExit(string direction, string locationKey) => Exits[direction.ToLower()] = locationKey;
    }

    public class Quest
    {
        public string Description { get; set; }
        public ICondition CompletionCondition { get; set; }
        public bool IsCompleted { get; set; }
    }
    #endregion

    #region 2.5-2.6 Условия и Эффекты
    public class HasItemCondition : ConditionBase
    {
        private string _item;
        public HasItemCondition(string item) => _item = item;
        public override bool Check(GameState state) => state.Player.Inventory.Contains(_item);
    }

    public class FlagCondition : ConditionBase
    {
        private string _flag;
        public FlagCondition(string flag) => _flag = flag;
        public override bool Check(GameState state) => state.Flags.ContainsKey(_flag) && state.Flags[_flag];
    }

    public class NotCondition : ConditionBase
    {
        private ICondition _inner;
        public NotCondition(ICondition inner) => _inner = inner;
        public override bool Check(GameState state) => !_inner.Check(state);
    }

    public class AndCondition : ConditionBase
    {
        private ICondition[] _conds;
        public AndCondition(params ICondition[] conds) => _conds = conds;
        public override bool Check(GameState state) => _conds.All(c => c.Check(state));
    }

    public class AddItemEffect : EffectBase
    {
        private string _item;
        public AddItemEffect(string item) => _item = item;
        public override void Apply(GameState state)
        {
            state.Player.Inventory.Add(_item);
            state.Log($"[Инвентарь] Получен предмет: {_item}", ConsoleColor.Green);
        }
    }

    public class DamageEffect : EffectBase
    {
        private int _dmg;
        public DamageEffect(int dmg) => _dmg = dmg;
        public override void Apply(GameState state)
        {
            state.Player.Health -= _dmg;
            state.Log($"[УРОН] Вы получили {_dmg} ед. урона! Здоровье: {state.Player.Health}", ConsoleColor.Red);
        }
    }

    public class LogEffect : EffectBase
    {
        private string _text;
        public LogEffect(string text) => _text = text;
        public override void Apply(GameState state) => state.Log(_text, ConsoleColor.Cyan);
    }

    public class SetFlagEffect : EffectBase
    {
        private string _flag;
        public SetFlagEffect(string flag) => _flag = flag;
        public override void Apply(GameState state) => state.Flags[_flag] = true;
    }

    public class AddExitEffect : EffectBase
    {
        private string _loc, _dir, _target;
        public AddExitEffect(string loc, string dir, string target) { _loc = loc; _dir = dir; _target = target; }
        public override void Apply(GameState state) => state.World[_loc].AddExit(_dir, _target);
    }
    #endregion

    #region 2.7 Объекты взаимодействия
    public class Chest : InteractableObject
    {
        public Chest(string name, string item)
        {
            Name = name;
            InteractionEffects.Add(new AddItemEffect(item));
            InteractionEffects.Add(new LogEffect($"Вы обыскали {name}."));
        }
    }

    public class Door : InteractableObject
    {
        public Door(string name, ICondition cond, IEffect effect)
        {
            Name = name;
            Conditions.Add(cond);
            InteractionEffects.Add(effect);
            InteractionEffects.Add(new LogEffect("Дверь открыта."));
        }
    }

    public class Terminal : InteractableObject
    {
        public Terminal(string name, ICondition cond, params IEffect[] effects)
        {
            Name = name;
            Conditions.Add(cond);
            InteractionEffects.AddRange(effects);
        }
    }
    #endregion

    #region 2.8 События
    public class OnEnterLocationEvent : GameEventBase { }
    public class OneTimeEvent : GameEventBase
    {
        private bool _fired = false;
        public override void TryTrigger(GameState state)
        {
            if (!_fired && Conditions.All(c => c.Check(state)))
            {
                _fired = true;
                foreach (var effect in Effects) effect.Apply(state);
            }
        }
    }
    #endregion

    #region 2.4 Команды
    public class LookCommand : CommandBase
    {
        public override void Execute(GameState state)
        {
            state.Log($"\n--- {state.CurrentLocation.Name} ---", ConsoleColor.White);
            state.Log(state.CurrentLocation.Description);
            state.Log("Выходы: " + string.Join(", ", state.CurrentLocation.Exits.Keys), ConsoleColor.DarkGray);
            if (state.CurrentLocation.Objects.Count > 0)
                state.Log("Объекты: " + string.Join(", ", state.CurrentLocation.Objects.Select(o => o.Name)), ConsoleColor.DarkGray);
        }
    }

    public class GoCommand : CommandBase
    {
        private string _dir;
        public GoCommand(string dir) => _dir = dir.ToLower();
        public override void Execute(GameState state)
        {
            if (state.CurrentLocation.Exits.TryGetValue(_dir, out string targetKey))
            {
                state.CurrentLocation = state.World[targetKey];
                state.Log($"Вы перешли в {state.CurrentLocation.Name}");
                foreach (var ev in state.CurrentLocation.Events.OfType<OnEnterLocationEvent>()) ev.TryTrigger(state);
            }
            else state.Log("Вы не можете туда пойти.", ConsoleColor.Red);
        }
    }

    public class InteractCommand : CommandBase
    {
        private string _obj;
        public InteractCommand(string obj) => _obj = obj.ToLower();
        public override void Execute(GameState state)
        {
            var obj = state.CurrentLocation.Objects.FirstOrDefault(o => o.Name.Equals(_obj, StringComparison.OrdinalIgnoreCase));
            if (obj != null) obj.Interact(state.Player, state);
            else state.Log("Здесь нет такого объекта.", ConsoleColor.Red);
        }
    }

    public class StatusCommand : CommandBase
    {
        public override void Execute(GameState state)
        {
            state.Log($"\n[СТАТУС] Здоровье: {state.Player.Health}", ConsoleColor.Magenta);
            state.Log($"[ИНВЕНТАРЬ] " + (state.Player.Inventory.Count > 0 ? string.Join(", ", state.Player.Inventory) : "Пусто"), ConsoleColor.Magenta);
            state.Log("[КВЕСТЫ]");
            foreach (var q in state.Quests) state.Log($"- {q.Description} [{(q.IsCompleted ? "Готово" : "В процессе")}]", ConsoleColor.DarkMagenta);
        }
    }

    public class HelpCommand : CommandBase
    {
        public override void Execute(GameState state) => 
            state.Log("Команды: look (осмотреться), go [направление], interact [объект], status (инвентарь), help, exit", ConsoleColor.Yellow);
    }
    #endregion

    #region Парсер и Главный цикл
    public class CommandParser
    {
        public ICommand Parse(string input)
        {
            var parts = input.ToLower().Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 0) return null;
            return parts[0] switch
            {
                "look" => new LookCommand(),
                "go" when parts.Length > 1 => new GoCommand(parts[1]),
                "interact" when parts.Length > 1 => new InteractCommand(parts[1]),
                "status" => new StatusCommand(),
                "help" => new HelpCommand(),
                _ => null
            };
        }
    }

    public class Game
    {
        private GameState _state = new GameState();
        private CommandParser _parser = new CommandParser();

        public void Init()
        {
            // 3. Создание локаций
            var hall = new Location { Name = "Hall", Description = "Стартовая зона. Безопасно. На севере видна запертая дверь (Door)." };
            var storage = new Location { Name = "Storage", Description = "Захламленное помещение склада. Всюду ящики." };
            var darkCorridor = new Location { Name = "DarkCorridor", Description = "Очень темный коридор. Ничего не видно." };
            var genRoom = new Location { Name = "GeneratorRoom", Description = "Комната с терминалом управления генератором." };
            var exitLoc = new Location { Name = "Exit", Description = "Финальный шлюз. Свобода!" };

            _state.World["Hall"] = hall;
            _state.World["Storage"] = storage;
            _state.World["DarkCorridor"] = darkCorridor;
            _state.World["GeneratorRoom"] = genRoom;
            _state.World["Exit"] = exitLoc;

            hall.AddExit("storage", "Storage");
            storage.AddExit("hall", "Hall");
            darkCorridor.AddExit("hall", "Hall");
            darkCorridor.AddExit("generator", "GeneratorRoom");
            genRoom.AddExit("corridor", "DarkCorridor");

            // 4.1 Механика: Темный коридор
            var darkEvent = new OnEnterLocationEvent();
            darkEvent.Conditions.Add(new NotCondition(new HasItemCondition("Torch")));
            darkEvent.Effects.Add(new DamageEffect(10));
            darkEvent.Effects.Add(new LogEffect("Вы споткнулись в темноте и поранились!"));
            darkCorridor.Events.Add(darkEvent);

            // 4.2 Механика: Запертая дверь
            hall.Objects.Add(new Door("Door", new HasItemCondition("Key"), new AddExitEffect("Hall", "north", "DarkCorridor")));

            // Наполнение склада
            storage.Objects.Add(new Chest("Box", "Key"));
            storage.Objects.Add(new Chest("Locker", "Torch"));
            storage.Objects.Add(new Chest("Crate", "Fuse"));
            storage.Objects.Add(new Chest("Toolbox", "Wrench"));

            // 4.4 Механика: Ловушка
            var trapEvent = new OneTimeEvent();
            trapEvent.Effects.Add(new DamageEffect(20));
            trapEvent.Effects.Add(new LogEffect("Сработала скрытая ловушка!"));
            storage.Events.Add(trapEvent);

            // 4.3 Механика: Генератор
            var genTerm = new Terminal("Terminal", 
                new AndCondition(new HasItemCondition("Fuse"), new HasItemCondition("Wrench")),
                new SetFlagEffect("GeneratorOn"),
                new AddExitEffect("GeneratorRoom", "exit", "Exit"),
                new LogEffect("Генератор загудел! Энергия подана на выходной шлюз."));
            genRoom.Objects.Add(genTerm);

            // Квесты
            _state.Quests.Add(new Quest { Description = "Включить генератор", CompletionCondition = new FlagCondition("GeneratorOn") });
            _state.Quests.Add(new Quest { Description = "Покинуть комплекс", CompletionCondition = new ConditionBase() });

            _state.CurrentLocation = hall;
            _state.Log("=== ИССЛЕДОВАТЕЛЬСКИЙ КОМПЛЕКС ===", ConsoleColor.White);
            _state.Log("Вы пришли в себя. Память фрагментарна. Нужно выбраться.");
        }

        public void Run()
        {
            while (_state.IsRunning)
            {
                // Проверка событий "каждый ход"
                foreach (var ev in _state.CurrentLocation.Events) ev.TryTrigger(_state);

                Console.Write("\n> ");
                string input = Console.ReadLine();
                if (input == "exit") break;

                var cmd = _parser.Parse(input);
                if (cmd != null) cmd.Execute(_state);
                else _state.Log("Неизвестная команда. Введите 'help'.", ConsoleColor.Red);

                // Проверка завершения квестов
                foreach (var q in _state.Quests.Where(x => !x.IsCompleted))
                {
                    if (q.CompletionCondition.Check(_state))
                    {
                        q.IsCompleted = true;
                        _state.Log($"[КВЕСТ ВЫПОЛНЕН]: {q.Description}", ConsoleColor.Green);
                    }
                }

                // Условия конца игры
                if (_state.CurrentLocation.Name == "Exit")
                {
                    _state.Log("\nПОЗДРАВЛЯЕМ! Вы выбрались живым!", ConsoleColor.Green);
                    _state.IsRunning = false;
                }
                if (_state.Player.Health <= 0)
                {
                    _state.Log("\nВы погибли в темных коридорах комплекса...", ConsoleColor.Red);
                    _state.IsRunning = false;
                }
            }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            Game game = new Game();
            game.Init();
            game.Run();
        }
    }
    #endregion
}
