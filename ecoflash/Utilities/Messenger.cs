using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using NLog;

using ecoflash.Utilities.Messages;

namespace ecoflash.Utilities
{
    public interface IMessage
    {
        string GetName();
    }

    public class Messenger
    {
        private static Messenger instance;

        private static Logger logger = LogManager.GetCurrentClassLogger();

        private Dictionary<string, List<ISubscriber>> map = new Dictionary<string, List<ISubscriber>>();

        private Messenger() { }

        public static Messenger Shared
        {
            get
            {
                if (instance == null)
                {
                    instance = new Messenger();
                }
                return instance;
            }
        }

        public void Subscribe(IMessage message, ISubscriber subscriber)
        {
            List<ISubscriber> subscribers = new List<ISubscriber>();

            if (map.ContainsKey(message.GetName()))
            {
                subscribers = map[message.GetName()];    
            }

            subscribers.Add(subscriber);
            map[message.GetName()] = subscribers;
        }

        public void Publish(IMessage message)
        {
            if (map.ContainsKey(message.GetName()))
            {
                List<ISubscriber> subscribers = map[message.GetName()];
                foreach (ISubscriber subscriber in subscribers)
                {
                    subscriber.OnMessageReceived(message);
                }
            }
        }
    }
}
